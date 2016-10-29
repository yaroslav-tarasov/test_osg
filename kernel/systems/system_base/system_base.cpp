#include "stdafx.h"

#include "kernel/object_class.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems.h"
#include "kernel/kernel.h"
#include "network/msg_dispatcher.h"
#include "common/locks.h"
#include "kernel/systems/impl/messages.h"
#include "kernel/msg_proxy.h"


#include "kernel/systems/impl/system_base.h"
#include "kernel/systems/ctrl_system.h"


#include "geometry/camera.h"

#include "common/randgen.h"

namespace kernel
{
    typedef cg::range_t<obj_id_t>   id_range_t;

    id_range_t make_id_range(size_t num, size_t total)
    {
        using binary::size_type;
        const size_type overall = std::numeric_limits<size_type>::max();

        auto len = overall / total;
        auto start = num * len;

        return id_range_t(start, start + len);
    }

    id_range_t& id_range()
    {
        static id_range_t range;
        return range;
    }

    //void update_id_range(msg_service& srv)
    //{
    //    id_range_t new_range = make_id_range(srv);

    //    if (id_range() != new_range)
    //        id_range() = new_range;
    //}

    void update_id_range(/*msg_service& srv*/)
    {
        id_range_t new_range = make_id_range(1000,10000);

        if (id_range() != new_range)
            id_range() = new_range;
    }


    //////////////////////////////////////////////////////////////////////////
    obj_create_data::obj_create_data(std::string const& hier_class, std::string const& name, dict_t&& own_data)
        : children_(data_.add_child("children"))
    {
        write(data_, hier_class, "hierarchy_class_name");
        write(data_, name      , "name");

        data_.add_child("own_data") = move(own_data);
    }

    obj_create_data::obj_create_data(std::string const& hier_class, std::string const& name)
        : children_(data_.add_child("children"))
    {
        write(data_, hier_class, "hierarchy_class_name");
        write(data_, name      , "name");
    }

    obj_create_data& obj_create_data::add_child(obj_create_data const& data)
    {
        children_.add_child("child_" + lexical_cast<string>(children_.children().size())) = data.dict();
        return *this;
    }

    obj_create_data& obj_create_data::add_data(object_data_t const& data)
    {
        write(data_, data      , "ext_data");
        return *this;
    }

    dict_t const& obj_create_data::dict() const
    {
        return data_;
    }

    //////////////////////////////////////////////////////////////////////////

 

struct system_base::msgs_blocker 
    : boost::noncopyable
{
    msgs_blocker(system_base& sb) : sb_(sb) { sb_.block_obj_msgs(true ); }
    ~msgs_blocker()                          { sb_.block_obj_msgs(false); }

private:
    system_base& sb_;
};

system_base::system_base(system_kind kind, msg_service& service, std::string const &objects_file_name )
    : kind_                 (kind)
    , msg_service_          (service, this)
    , create_object_lock_   (false)
    , id_randgen_           (randgen_seed_tag())
    , udp_messages_size_    (0)
    , udp_msg_threshold_    (1300 ) // limit for Win32
    , block_obj_msgs_counter_(0)
{
    // загружаем файл objects.xml - в нем дерево подсистем и объектов, в том числе ссылки на внешние файлы ani, fpl, bada
    tinyxml2::XMLDocument units_doc;
    //fs::path objects_xml = fs::path(cfg().path.data) / objects_file_name; 
    //Verify(is_regular_file(objects_xml));

    xinclude_load_file(units_doc, objects_file_name/*objects_xml.string()*/, 2);
    // строим дерево 
    root_ =     create_object_class(units_doc.RootElement());

    msg_disp_
        .add<msg::object_created>(boost::bind(&system_base::on_object_created, this, _1))
        .add<msg::destroy_object>(boost::bind(&system_base::on_object_destroy, this, _1))
        .add<msg::object_msg    >(boost::bind(&system_base::on_object_msg    , this, _1))
        .add<msg::container_msg >(boost::bind(&system_base::on_container_msg , this, _1));

    //update_id_range(service);
    update_id_range();
}

system_base::~system_base()
{
}

system_kind system_base::kind() const
{
    return kind_;
}

void system_base::update (double time)
{
    update_time_ = time;

    do_pre_update   (time);


    do_update       (time);


    do_post_update  (time);


    // call update atc
    //if (!last_update_atc_ || time - *last_update_atc_ >= atc_update_period())
    //{
    //    do_update_atc(time);
    //    last_update_atc_ = size_t(time / atc_update_period()) * atc_update_period(); 
    //}

    update_time_.reset();
    last_update_time_ = time;

    // sending pending messages 
    if (!tcp_messages_.empty())
        msg_service_((system_kind)kind_, network::wrap_msg(msg::container_msg(move(tcp_messages_))), true );


    if (!udp_messages_.empty())
    {
        msg_service_((system_kind)kind_, network::wrap_msg(msg::container_msg(move(udp_messages_))), false);
        udp_messages_size_ = 0;
    }


}

void system_base::on_msg(binary::bytes_cref bytes)
{
    FIXME (Проскакивает такая хрень)
    if(bytes.size()>0)
        msg_disp_.dispatch_bytes(bytes);
}

//! базовая функция загрузки упражнения
void system_base::load_exercise(dict_cref dict)
{
    //profiler::reset();

    LogTrace("Loading exercise by " << sys_name(kind_));
    time_counter tc;

    for (auto it = objects_.begin(); it != objects_.end(); ++it)
        base_presentation_ptr(it->second.lock())->reset_parent();
    root_objects_.clear();
    objects_     .clear();

    dict_t const* objs = dict.find("objects");
    Assert(objs);

    // loading objects in proper order 
    // first: auto objects (according to their dependencies), later all other objects
    // "автоматические" объекты
    std::map<string, const dict_t*> auto_objects;
    // обычные (остальные) объекты
    vector<const dict_t*>           usual_objects;

    // проход по "объектам" упражнения, загрузка
    for (auto it = objs->children().begin(); it != objs->children().end(); ++it)
    {
        auto class_obj = root_->find_class(dict::read_dict<string>(it->second, "hierarchy_class_name"));

        if (class_obj->check_attribute("auto", "true"))
            auto_objects[class_obj->name()] = &(it->second);
        else 
            usual_objects.push_back(&(it->second));
    }

    // некая сортировка объектов с учетом их зависимостей(???)
    vector<string> const& ordered_auto_objects = auto_object_order();

    //profiler::add_dsc("preparation");
    // создание авто объектов, загрузка иерархий
    for (auto it = ordered_auto_objects.begin(); it != ordered_auto_objects.end(); ++it)
    {
        auto dsc = auto_objects.find(*it); 

        if (dsc != auto_objects.end())
            load_object_hierarchy(*(dsc->second));

        //profiler::add_dsc(("loading " + *it).c_str());
    }

    // создание и загрузка обычных объектов
    for (auto it = usual_objects.begin(); it != usual_objects.end(); ++it)
    {
        dict_t const& d = *(*it);
        string name = dict::read_dict<string>(d, "name");

        load_object_hierarchy(d);
        //profiler::add_dsc(("loading " + name).c_str());
    }

    //exercise_loaded_signal_();
    //profiler::add_dsc("signal firing");
    LogInfo("Exercise loaded in " << tc.to_double(tc.time()) << " seconds");

    //profiler::out_times((sys_name(kind_) + " sys: loading exercise").c_str());
}

void system_base::save_exercise(dict_ref dict, bool safe_key) const
{
    time_counter tc;

    dict_t& objs = dict.add_child("objects");

    for (auto it = root_objects_.begin(); it != root_objects_.end(); ++it)
    {
        object_info_ptr info = it->second;
        string branch_name = info->name() + "_" + lexical_cast<string>(info->object_id());

        save_object_hierarchy(it->second, objs.add_child(branch_name.c_str()), safe_key);
    }

    LogInfo("Exercise saved in " << tc.to_double(tc.time()));
}

optional<double>  system_base::update_time() const
{
    return update_time_;
}

optional<double> system_base::last_update_time() const
{
    return last_update_time_;    
}

double  system_base::atc_update_period() const
{
    FIXME(И ведь везде хард код)
    return 4.;
}

system_base::objects_t const& system_base::root_objects() const
{
    return root_objects_;
}

object_info_ptr system_base::get_object( obj_id_t object_id ) const
{
    auto it = objects_.find(object_id);
    if (it == objects_.end())
        return object_info_ptr();

    return object_info_ptr(it->second);
}

void system_base::destroy_object( obj_id_t object_id )
{ 
    // destroy self
    msg_service_((system_kind)kind_, network::wrap_msg(msg::destroy_object(object_id)), true);
    process_destroy_object(object_id);
}

void system_base::register_obj_id(obj_id_t id)
{
    used_ids_.insert(id);
}

auto system_base::generate_object_id() -> obj_id_t
{
    auto r = id_range();
    obj_id_t id = r.lo();

    do
    {
        id = id_randgen_(r.lo(), r.hi());
    }
    while(used_ids_.count(id) != 0);

    return id;
}

void system_base::on_session_loaded() 
{
    session_loaded_signal_() ;
}

void system_base::on_session_stopped()
{
    session_stopped_signal_() ;

    vector<obj_id_t> roots = remove_roots_order();

    for (auto it = roots.begin(); it != roots.end(); ++it)
        if (objects_.count(*it) != 0) // e.g. fpl_manager removes corresponding aircraft on fpl destroying 
            process_destroy_object(*it);

    Assert(root_objects_.empty());
}

void system_base::on_time_factor_changed (double time, double factor)
{
    // it allows to restart atc_update after time changing (e.g. while moving slider in history)
    last_update_atc_ = size_t(time / atc_update_period()) * atc_update_period(); 
    time_factor_changed_signal_(time, factor);
}

object_info_ptr system_base::create_object(object_class_ptr hier_class, std::string const &obj_name)
{
    object_info_ptr obj;
    msgs_blocker    mb(*this);

    {
        locks::bool_lock l(create_object_lock_);
        obj = create_object_hierarchy_impl(hier_class, obj_name, true);
    }

    if (obj)
        fire_object_created(obj);

    return obj;
}

object_info_ptr system_base::local_create_object(obj_create_data const& data)
{
	object_info_ptr obj;
	msgs_blocker    mb(*this);

	{
		locks::bool_lock l(create_object_lock_,false);
		obj = load_object_hierarchy_impl(object_class_ptr(), data.dict(), true, false);
	}
        
    auto d = object_data_ptr(obj)->get_data();

	// fire only on my system
	if (obj)
		object_created_signal_(obj);

	return obj;
}

void system_base::pack_exercise(const creating_objects_list_t& co , dict_ref dict, bool safe_key) /*const*/
{
	time_counter tc;

	dict_t& objs = dict.add_child("objects");

	for (auto it = co.begin(); it != co.end(); ++it)
	{
		object_info_ptr info = local_create_object(*it);

		string branch_name = info->name() + "_" + lexical_cast<string>(info->object_id());

		save_object_hierarchy(info, objs.add_child(branch_name.c_str()), safe_key);
	}

	LogInfo("Exercise saved in " << tc.to_double(tc.time()));
}

object_info_ptr system_base::create_object(obj_create_data const& data)
{
    object_info_ptr obj;
    msgs_blocker    mb(*this);

    {
        locks::bool_lock l(create_object_lock_,false);
        obj = load_object_hierarchy_impl(object_class_ptr(), data.dict(), true, false);
    }

    if (obj)
        fire_object_created(obj);

    return obj;
}

object_info_ptr system_base::load_object_hierarchy(dict_t const& dict)
{                                                                                                                                                                       
    object_info_ptr obj;
    msgs_blocker    mb(*this);

    {
        locks::bool_lock l(create_object_lock_);
        obj = load_object_hierarchy_impl(object_class_ptr(), dict, true, true);
    }

    // fire only on my system
    if (obj)
        object_created_signal_(obj);

    return obj;
}

void system_base::save_object_hierarchy(object_info_ptr objinfo, dict_t& dict, bool safe_key) const
{
    //     optional<time_counter> tc;
    //     if (objinfo->parent().expired())
    //         tc = in_place();
    
    dict_t d;
    
    write(dict, objinfo->hierarchy_class()->name(), "hierarchy_class_name");
    write(dict, objinfo->name()     , "name");
    write(dict, objinfo->object_id(), "id"  );
    write(dict, object_data_ptr(objinfo)->get_data(), "ext_data"  );

    dict_t& children = dict.add_child("children");

    size_t count = 0;
    for (object_info_vector::const_iterator it = objinfo->objects().begin(); it != objinfo->objects().end(); ++it)
        save_object_hierarchy(*it, children.add_child("child_" + lexical_cast<string>(count++)), safe_key);

    objinfo->save(dict.add_child("own_data"), safe_key);

    //     if (tc)
    //         LogInfo("Saving " << objinfo->name() << "; time: " << tc->to_double(tc->time()));
}

FIXME(А это должно быть в отдельном файле)

inline object_info_ptr create_object(kernel::object_create_t oc, dict_copt dict = boost::none)
{   
    string lib_name   = *oc.hierarchy_class->find_attribute("lib");
    string class_name = *oc.hierarchy_class->find_attribute("cpp_class");

    string function_name;
    if (auto f_name = oc.hierarchy_class->find_attribute(sys_name(oc.sys->kind()) + "_cpp_class"))
        function_name = *f_name;
    else
    {
        function_name = class_name + "_" + sys_name(oc.sys->kind());
    }

    //auto fp = fn_reg::function<object_info_ptr(kernel::object_create_t const&)>(function_name);

    /*    if(fp)
    return fp(oc);
    else*/ if(auto fp_d = fn_reg::function<object_info_ptr(kernel::object_create_t const&, dict_copt dict)>(function_name))
        return fp_d(oc,dict);
    else
        return fn_reg::function<object_info_ptr(kernel::object_create_t const&, dict_copt)>(/*lib_name,*/ class_name+"_view")(boost::cref(oc), dict);


    return object_info_ptr();
}


object_info_ptr system_base::create_object_hierarchy_impl(
    object_class_ptr   hierarchy_class, 
    std::string const& name, 
    bool               is_root)

{
    auto lib_name       = hierarchy_class->find_attribute("lib");
    auto cpp_class_name = hierarchy_class->find_attribute("cpp_class");

    if (lib_name && cpp_class_name)
    {
        object_info_vector children;

        // create children
        for (auto it = hierarchy_class->classes().begin(); it != hierarchy_class->classes().end(); ++it)
            if(object_info_ptr child = create_object_hierarchy_impl(*it, (*it)->name(), false))
                children.push_back(child);

        // create self
        size_t id = generate_object_id();
        VerifyMsg(objects_.find(id) == objects_.end(), "Duplicate object id");

        auto msg_service = boost::bind(&system_base::send_obj_message, this, id, _1, _2, _3);
        auto block_msgs  = [this](bool block){ block_obj_msgs(block); };

        kernel::object_create_t oc(hierarchy_class, this, id, name, children, msg_service, block_msgs, 0);

        // создаем объект через динамическую загрузку библиотеки
        object_info_ptr object = kernel::create_object(oc);
        if (object == nullptr)
        {
            LogWarn("Cannot create object " << cpp_class_name << " or object creation was interrupted");
            return object;
        }

        register_obj_id(id);

        for (auto it = children.begin(); it != children.end(); ++it)
            base_presentation_ptr(*it)->reset_parent(object);

        if (is_root)
        {
            base_presentation_ptr(object)->reset_parent();
            root_objects_.insert(std::make_pair(id, object));

            visit_all_hierarchy(object, [this](object_info_ptr ptr){ this->objects_.insert(make_pair(ptr->object_id(), ptr)); });
        }

        return object;
    }
    return object_info_ptr();
}

object_info_ptr system_base::load_object_hierarchy_impl(
    object_class_ptr      parent_hierarchy_class, 
    dict_cref             dict, 
    bool                  is_root, 
    bool                  read_id)
{
    //     optional<time_counter> tc;
    //     if (is_root)
    //         tc = in_place();

    using dict::read_dict;

    string class_name = read_dict<string>(dict, "hierarchy_class_name");
    string name       = read_dict<string>(dict, "name"                );
    uint32_t id       = read_id ? read_dict<uint32_t>(dict, "id") : generate_object_id();
    
    auto ext_data = dict.find("ext_data");
    uint32_t ext_id   = ext_data ? read_dict<uint32_t>(dict, "ext_data") : 0;

    object_class_ptr hierarchy_class =         
        parent_hierarchy_class 
        ? parent_hierarchy_class->find_class(class_name) 
        : root_->find_class(class_name);

    Assert(hierarchy_class);

    // load children
    object_info_vector children;

    auto objects = dict.find("children");
    Assert(objects);

    for (auto it = objects->children().begin(); it != objects->children().end(); ++it)
    {
        if (object_info_ptr child = load_object_hierarchy_impl(hierarchy_class, it->second, false, read_id))
            children.push_back(child);
    }


    if (read_id && objects_.count(id) != 0)
    {
        LogError("Creating an object with already existing id! Are you creating object while playing history?");
        return nullptr;
    }

    string lib_name       = *hierarchy_class->find_attribute("lib");
    string cpp_class_name = *hierarchy_class->find_attribute("cpp_class");

    // create self
    auto msg_service = boost::bind(&system_base::send_obj_message, this, id, _1, _2, _3);
    auto block_msgs  = [this](bool block){ block_obj_msgs(block); };


    kernel::object_create_t oc(hierarchy_class, this, id, name, children, msg_service, block_msgs, ext_id);

    auto ownd_data = dict.find("own_data");

    object_info_ptr object = kernel::create_object(oc , dict_copt(ownd_data!=nullptr, *ownd_data));

    if (object == nullptr)
    {
        LogError("Unable to load object " << name);
        return object;
    }

    register_obj_id(id);

    for (auto it = children.begin(); it != children.end(); ++it)
        base_presentation_ptr(*it)->reset_parent(object);

    if (is_root)
    {
        base_presentation_ptr(object)->reset_parent();
        root_objects_.insert(std::make_pair(id, object));

        visit_all_hierarchy(
            object, 
            [this](object_info_ptr ptr)
        { 
            this->objects_.insert(make_pair(ptr->object_id(), ptr)); 
        });

        //LogInfo("Loading " << name << "; time: " << tc->to_double(tc->time()));
    }

    return object;
}

object_class_vector const& system_base::object_classes() const
{
    return root_->classes();
}

object_class_ptr system_base::get_object_class(std::string const& name) const
{
    return root_->find_class(name);
}

std::string system_base::generate_unique_name(std::string const &init_name) const
{
    size_t index = 0;
    std::string unique_name;

    std::set<std::string> names;

    for ( auto it = root_objects_.begin(), end = root_objects_.end(); it != end; ++it)
        if ( boost::starts_with(it->second->name(), init_name) )
            names.insert(it->second->name());

    while (names.end() != names.find(unique_name = str(cpp_utils::inplace_format("%s %d") % init_name % index++))) {}

    return unique_name;
}

void system_base::fire_object_created(object_info_ptr obj)
{
    //send to other systems
    dict_t dic;
    save_object_hierarchy(obj, dic, false);

    msg_service_((system_kind)kind_, network::wrap_msg(msg::object_created(binary::wrap(std::move(dic)))), true);

    //fire on my system
    object_created_signal_(obj);
}

//////////////////////////////////////////////////////////////////////////
// msg processing 
void system_base::on_object_created(msg::object_created const& msg)
{
    dict_t dict;
    binary::unwrap(msg.data, dict);

    load_object_hierarchy(dict);
}

void system_base::on_object_destroy(msg::destroy_object const& msg)
{
    process_destroy_object(msg.obj_id);
}

void system_base::on_object_msg(msg::object_msg const& msg)
{
    auto it = objects_.find(msg.object_id);
    if (it == objects_.end())
    {
        Assert(destroyed_objects_.count(msg.object_id) != 0);
        return;
    }

    // see comments for base_view_presentation::set function 
    optional<msgs_blocker> mb;
    bool top_level_handler = false;
    if (!msg.just_cmd)
    {
        if (!obj_msgs_blocked())
            top_level_handler = true;

        mb = in_place(boost::ref(*this));
    }

    base_presentation_ptr(it->second.lock())->on_msg(msg.data);

    // comparing messages protocol for incoming message and outgoing messages 
    if (top_level_handler && kind_ == sys_model)
    {
        msg_protocol_t only_theirs;
        set_difference(
            msg.msg_protocol_.begin(), msg.msg_protocol_.end(),
            msg_protocol_   ->begin(), msg_protocol_   ->end(), 
            std::inserter(only_theirs, only_theirs.begin()));

        while(!only_theirs.empty())
        {
            auto msg = *only_theirs.begin();
            auto obj = get_object(msg::details::get_obj_id(msg));

            LogError("Found excess incoming message for obj " << obj->name() << "; msg id " << msg::details::get_msg_id(msg));
            only_theirs.erase(only_theirs.begin());
        }

        msg_protocol_t only_ours;
        set_difference(
            msg_protocol_   ->begin(), msg_protocol_   ->end(), 
            msg.msg_protocol_.begin(), msg.msg_protocol_.end(),
            inserter(only_ours, only_ours.begin()));

        while(!only_ours.empty())
        {
            auto msg = *only_ours.begin();
            auto obj = get_object(msg::details::get_obj_id(msg));

            LogError("Found excess outgoing from model message for obj " << obj->name() << "; msg id " << msg::details::get_msg_id(msg));
            only_ours.erase(only_ours.begin());
        }
    }
}

void system_base::on_container_msg(msg::container_msg const& msg)
{                                                      
    for (size_t i = 0; i < msg.msgs.size(); ++i)
        msg_disp_.dispatch_bytes(msg.msgs[i]);
}

void system_base::do_pre_update(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
        base_presentation_ptr(it->second)->pre_update(time);
}

void system_base::do_update(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
    {
        base_presentation_ptr(it->second)->update(time);

    }
}

void system_base::do_post_update(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
        base_presentation_ptr(it->second)->post_update(time);
}

void system_base::do_update_atc(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
        base_presentation_ptr(it->second)->update_atc(time);
}

network::msg_dispatcher<>& system_base::msg_disp()
{
    return msg_disp_;
}

void system_base::block_obj_msgs(bool block)
{
    if (block)
    {
        if (block_obj_msgs_counter_ == 0)
            // this could happen via sending message, or via handling incoming one
            msg_protocol_ = in_place();

        ++block_obj_msgs_counter_;
    }
    else 
    {
        Assert(block_obj_msgs_counter_ > 0);
        --block_obj_msgs_counter_;
    }

}

void system_base::send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd)
{
    if (obj_msgs_blocked())
    {
        if (msg_protocol_ && !just_cmd)
        {
            binary::input_stream is(bytes);
            uint32_t msg_id = network::read_id(is);

            auto res = msg_protocol_->insert(msg::details::make_msg_obj_id(object_id, msg_id));

            // doesn't work because of nodes_manager  
            //             if (!res.second)
            //             {
            //                 auto obj = get_object(object_id);
            //                 VerifyMsg(res.second, "Object " << obj->name() << " tries to send msg with id " << msg_id << " several times from " << sys_name(kind_));
            //             }
        }

        return;
    }

    // prevent from sending messaged to destroying objects
    if (obj_ids_to_destroy_.count(object_id) != 0)
        return;

    binary::bytes_t msg = network::wrap_msg(msg::object_msg(object_id, bytes, just_cmd, msg_protocol_ ? move(*msg_protocol_) : msg_protocol_t()));
    msg_protocol_.reset();

    if (update_time())
    {
        if (sure)
            tcp_messages_.push_back(move(msg));
        else 
        {
            if (udp_messages_size_ + binary::size(msg) > udp_msg_threshold_)
            {                                           
                if (!udp_messages_.empty())
                {
                    msg_service_((system_kind)kind_, network::wrap_msg(msg::container_msg(move(udp_messages_))), false);
                    udp_messages_size_ = 0;
                }

                Assert(udp_messages_.empty());
            }

            udp_messages_size_ += binary::size(msg);
            udp_messages_.push_back(move(msg));
        }
    }
    else 
    {   
        msg_service_((system_kind)kind_, msg, sure);
    }
}

void system_base::process_destroy_object( size_t object_id )
{
    auto it = objects_.find(object_id);
    if(it == objects_.end())
    {
        LogError("Destroying already destroyed object. Are you destroying object while playing history?  oid = " << object_id);
        return;
    }

    object_info_wptr to_destroy = it->second;

    std::vector<object_info_wptr> objs_to_destroy;
    std::vector<obj_id_t> obj_ids_to_destroy;

    visit_all_hierarchy(object_info_ptr(to_destroy), 
        [this, &obj_ids_to_destroy, &objs_to_destroy](object_info_ptr obj) 
    {                 
        base_presentation_ptr child(obj);
        child->reset_parent(); // to correct release pointers to parent (prevent cross pointers with parent)

        obj_ids_to_destroy.push_back(obj->object_id());
        objs_to_destroy.push_back(obj);
    });

    // prevent from sending messages to the destroying object 
    obj_ids_to_destroy_.insert(obj_ids_to_destroy.begin(), obj_ids_to_destroy.end());
    destroyed_objects_ .insert(obj_ids_to_destroy.begin(), obj_ids_to_destroy.end());

    for (size_t i = 0; i < obj_ids_to_destroy.size(); ++i)
    {
        auto jt = objects_.find(obj_ids_to_destroy[i]);
        Assert(jt != objects_.end());
        objects_.erase(jt);
    }

    auto jt = root_objects_.find(object_id);
    Assert(jt != root_objects_.end());

    // save object to avoid problem with call to destroy from object_destroying_signal_
    object_info_ptr to_destroy_ptr = jt->second ;
    root_objects_.erase(jt);

    object_destroying_signal_(to_destroy_ptr);

    // now the whole object hierarchy is destroyed - no more need to watch for these ids
    for (auto it = obj_ids_to_destroy.begin(); it != obj_ids_to_destroy.end(); ++it)
        obj_ids_to_destroy_.erase(*it);

    // check correct destroying 
    to_destroy_ptr.reset();

    check_destroy(objs_to_destroy);
}
vector<string> const& system_base::auto_object_order()
{
    static vector<string> res;

    if (!res.empty())
        return res;

    // topology sort for dependencies 
    std::set<object_class_ptr> to_create;
    std::set<object_class_ptr> creating;
    std::set<object_class_ptr> created_objs; 

    for(auto it = root_->classes().begin(); it != root_->classes().end(); ++it)
        if((*it)->check_attribute("auto", "true"))
            to_create.insert(*it);

    std::stack<object_class_ptr>  st;

    res.reserve(to_create.size());
    while(!to_create.empty())
    {      
        auto pitem = to_create.begin();

        st.push(*pitem);       

        creating .insert(*pitem);
        to_create.erase (pitem);

        while(!st.empty())
        {
            object_class_ptr ptr = st.top();
            auto depends = ptr->find_attribute("depends_on");

            bool no_unproceed_deps = true;
            if (depends)
            {
                using namespace boost;

                vector<string> objects;
                split(objects, *depends, is_space(), token_compress_on);

                for (size_t i = 0; i != objects.size(); ++i)
                {
                    auto dep = root_->find_class(objects[i]);
                    VerifyMsg(dep, "Invalid dependent class \'" << objects[i] << "\' mentioned for \'" << ptr->name() << "\'");

                    if  (created_objs.count(dep) == 0)
                    {
                        VerifyMsg(creating.count(dep) == 0, "Circular dependency found on \'" << dep->name() << "\'");

                        st.push(dep);

                        creating .insert(dep);
                        to_create.erase (dep);

                        no_unproceed_deps = false;
                    }
                }
            }

            if (no_unproceed_deps)
            {
                st.pop();

                if (created_objs.count(ptr) == 0)
                {
                    res.push_back(ptr->name());

                    creating    .erase (ptr);
                    created_objs.insert(ptr);
                }
            }
        }
    }

    return res;
}

vector<obj_id_t> system_base::remove_roots_order()
{
    vector<obj_id_t> res;
    res.reserve(root_objects_.size());

    // removing objects in proper order 
    std::map<string, object_info_ptr> auto_objects;
    vector<object_info_ptr>           usual_objects;

    for (auto it = root_objects_.begin(); it != root_objects_.end(); ++it)
    {
        auto class_obj = it->second->hierarchy_class();

        if (class_obj->check_attribute("auto", "true"))
            auto_objects[class_obj->name()] = it->second;
        else 
            usual_objects.push_back(it->second);
    }

    for (auto it = usual_objects.begin(); it != usual_objects.end(); ++it)
        res.push_back((*it)->object_id());

    // некая сортировка объектов с учетом их зависимостей(???)
    vector<string> const& ordered_auto_objects = auto_object_order();
    // reverse order 
    for (auto it = ordered_auto_objects.rbegin(); it != ordered_auto_objects.rend(); ++it)
    {
        auto dsc = auto_objects.find(*it); 

        if (dsc != auto_objects.end())
            res.push_back(dsc->second->object_id());
    }

    return res;
}

void system_base::check_destroy(std::vector<object_info_wptr> const& objs_to_destroy)
{
    for (auto it = objs_to_destroy.begin(); it != objs_to_destroy.end(); ++it)
    {
        if (!it->expired())
        {
            size_t use_count = it->use_count();
            string name = it->lock()->name();
            LogTrace("object (id = " << it->lock()->object_id() << ", name = " << name << ", sys = " << sys_name(kind()) << ") can't be destroyed  (use_count = " << use_count << ")");
            {
                force_log fl;
                LOG_ODS_MSG ("object (id = " << it->lock()->object_id() << ", name = " << name << ", sys = " << sys_name(kind()) << ") can't be destroyed  (use_count = " << use_count << ") \n");
            }


            vector<obj_id_t> roots = remove_roots_order();

            for (auto obj_it = roots.begin(); obj_it != roots.end(); ++obj_it)
            {
                string name2;
                uint32_t id2 = *obj_it;

                if (object_info_ptr obj = get_object(id2))
                    name2 = obj->name();
                else
                    continue;

                process_destroy_object(id2);

                if (use_count != it->use_count())
                {
                    LogTrace("object (id = " << id2 << ", name = " << name2 << ") has references to " << name);
                    {
                        force_log fl;
                        LOG_ODS_MSG ("object (id = " << id2 << ", name = " << name2 << ") has references to " << name);
                    }

                    use_count = it->use_count();
                }
            }

            Verify(0);
        }
    }
}

bool system_base::obj_msgs_blocked() const
{
    return block_obj_msgs_counter_ > 0;
}






} // kernel