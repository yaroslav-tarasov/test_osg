#include "stdafx.h"

#include "kernel/object_class.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems.h"
#include "kernel/kernel.h"
#include "network/msg_dispatcher.h"
#include "common/locks.h"
#include "kernel/systems/impl/messages.h"
#include "kernel/msg_proxy.h"

#include "kernel/systems/fake_system.h"

#include "kernel/systems/vis_system.h"
#include "kernel/systems/ctrl_system.h"

#include "kernel/systems/visual_system/visual_object_impl.h"

#include "geometry/camera.h"


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

    dict_t const& obj_create_data::dict() const
    {
        return data_;
    }

    //////////////////////////////////////////////////////////////////////////


struct model_system_impl;
struct visual_system_impl;
struct ctrl_system_impl;

system_ptr create_model_system(msg_service& service, std::string const& script) 
{
    LogInfo("Creating MODEL system");
    return kernel::system_ptr(boost::make_shared<model_system_impl>(boost::ref(service), boost::ref(script)));
}

system_ptr create_visual_system(msg_service& service, av::IVisualPtr vis, vis_sys_props const& vsp ) 
{
    LogInfo("Creating VISUAL system");
    return kernel::system_ptr(boost::make_shared<visual_system_impl>(boost::ref(service),  vis, boost::ref(vsp)));
}

system_ptr create_ctrl_system( msg_service& service ) 
{
    LogInfo("Creating CTRL system");
    return kernel::system_ptr(boost::make_shared<ctrl_system_impl>(boost::ref(service)));
}

struct  fake_system_base
    : system            
    , system_session    
    //, objects_factory  
    , fake_objects_factory
    , object_collection 
    , boost::enable_shared_from_this<fake_system_base>
{
    fake_system_base(system_kind kind, msg_service& service, std::string const &objects_file_name);
    //! деструктор
    ~fake_system_base();
    // system
protected:
    system_kind         kind                () const                    override;
    void                update              (double time)               override;
    void                on_msg              (binary::bytes_cref bytes)  override;
    void                load_exercise       (dict_cref dict)            override;
    void                save_exercise       (dict_ref  dict, bool safe_key) const      override;
    optional<double>    update_time         () const                    override;
    optional<double>    last_update_time    () const                    override;
    double              atc_update_period   () const                    override;

    // system_session
protected:
    void on_session_loaded      ();
    void on_session_stopped     ();
    void on_time_factor_changed (double time, double factor);

    // objects_factory
protected:
    object_info_ptr create_object            (object_class_ptr hierarchy_class, std::string const &name)     override;
    object_info_ptr create_object            (obj_create_data const& descr)                                  override;
    object_info_ptr load_object_hierarchy    (dict_cref dict)                                                override;
    void            save_object_hierarchy    (object_info_ptr objinfo, dict_ref dict, bool safe_key) const   override;    
    object_class_vector const& object_classes() const                                                   override;
    object_class_ptr get_object_class(std::string const& name) const                                    override;

    std::string generate_unique_name(std::string const &init_name) const override;

private:
    void check_destroy(std::vector<object_info_wptr> const& objs_to_destroy);

    // object_collection
protected:
    objects_t const&    root_objects    () const override;
    object_info_ptr     get_object      (obj_id_t object_id) const override;
    void                destroy_object  (obj_id_t object_id) override;

protected:
    void send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd);

protected:
    void process_destroy_object( size_t object_id );

protected:
    virtual void do_pre_update  (double time);
    virtual void do_update      (double time);
    virtual void do_post_update (double time);
    virtual void do_update_atc  (double time);

protected:
    network::msg_dispatcher<>&  msg_disp();

protected:
    typedef 
        ph_map<size_t, object_info_wptr>::map_t
        weak_objects_t;

    //! дерево объектов (физически map)
    objects_t       root_objects_; // roots
    //! линейный набор (map) ссылок на объекты
    weak_objects_t  objects_     ; // plain object list 

protected:
//     typedef weak_ptr<msg_keeper>    msg_keeper_ptr;
//     typedef list<msg_keeper_ptr>    msg_keeper_ptrs; 

    object_info_ptr create_object_hierarchy_impl(object_class_ptr hierarchy_class       , std::string const &name, bool is_root);
    object_info_ptr load_object_hierarchy_impl  (object_class_ptr parent_hierarchy_class, dict_cref dict, bool is_root, bool read_id);

protected:
    vector<string> const&   auto_object_order ();
    vector<obj_id_t>        remove_roots_order();

protected:
    typedef ph_set<obj_id_t>::set_t obj_set_t;

protected:
    system_kind     kind_;
    msg_service_reg msg_service_;

    optional<double> update_time_;
    optional<double> last_update_time_;
    optional<double> last_update_atc_;

protected:
    object_class_ptr root_;

private:
    // we do not want to assign equal objects id to previously created object
    // even if it has been already removed - such a strategy makes history recording easier 
    obj_set_t used_ids_;
    randgen<> id_randgen_;

private:
    void     register_obj_id   (obj_id_t id);
    obj_id_t generate_object_id();

private:
    void fire_object_created(object_info_ptr obj);

private:
    void on_object_created(msg::object_created const& msg);
    void on_object_destroy(msg::destroy_object const& msg);
    void on_object_msg    (msg::object_msg     const& msg);
    void on_container_msg (msg::container_msg  const& msg);

private:
    struct msgs_blocker;
    void block_obj_msgs  (bool block);
    bool obj_msgs_blocked() const;

private:
    //! диспетчер сообщений для всех объектов "подсистем"
    network::msg_dispatcher<> msg_disp_;

private:
    msg::container_msg::msgs_t tcp_messages_;
    msg::container_msg::msgs_t udp_messages_;

    size_t       udp_messages_size_;
    const size_t udp_msg_threshold_;

private:
    obj_set_t obj_ids_to_destroy_;
    obj_set_t destroyed_objects_;

private:
    bool create_object_lock_;

private:
    size_t       block_obj_msgs_counter_;

private:
    typedef msg::object_msg::msg_protocol_t msg_protocol_t;

    optional<msg_protocol_t> msg_protocol_;

};  

struct fake_system_base::msgs_blocker 
    : boost::noncopyable
{
    msgs_blocker(fake_system_base& sb) : sb_(sb) { sb_.block_obj_msgs(true ); }
    ~msgs_blocker()                          { sb_.block_obj_msgs(false); }

private:
    fake_system_base& sb_;
};

fake_system_base::fake_system_base(system_kind kind, msg_service& service, std::string const &objects_file_name )
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
        .add<msg::object_created>(boost::bind(&fake_system_base::on_object_created, this, _1))
        .add<msg::destroy_object>(boost::bind(&fake_system_base::on_object_destroy, this, _1))
        .add<msg::object_msg    >(boost::bind(&fake_system_base::on_object_msg    , this, _1))
        .add<msg::container_msg >(boost::bind(&fake_system_base::on_container_msg , this, _1));

    //update_id_range(service);
    update_id_range();
}

fake_system_base::~fake_system_base()
{
}

system_kind fake_system_base::kind() const
{
    return kind_;
}

void fake_system_base::update (double time)
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

void fake_system_base::on_msg(binary::bytes_cref bytes)
{
    FIXME (Проскакивает такая хрень)
    if(bytes.size()>0)
        msg_disp_.dispatch_bytes(bytes);
}

//! базовая функция загрузки упражнения
void fake_system_base::load_exercise(dict_cref dict)
{   
	exercise_loaded_signal_();
}

void fake_system_base::save_exercise(dict_ref dict, bool safe_key) const
{
}

optional<double>  fake_system_base::update_time() const
{
    return update_time_;
}

optional<double> fake_system_base::last_update_time() const
{
    return last_update_time_;    
}

double  fake_system_base::atc_update_period() const
{
    FIXME(И ведь везде хард код)
    return 4.;
}

fake_system_base::objects_t const& fake_system_base::root_objects() const
{
    return root_objects_;
}

object_info_ptr fake_system_base::get_object( obj_id_t object_id ) const
{
    auto it = objects_.find(object_id);
    if (it == objects_.end())
        return object_info_ptr();

    return object_info_ptr(it->second);
}

void fake_system_base::destroy_object( obj_id_t object_id )
{ 
    // destroy self
    msg_service_((system_kind)kind_, network::wrap_msg(msg::destroy_object(object_id)), true);
    process_destroy_object(object_id);
}

void fake_system_base::register_obj_id(obj_id_t id)
{
    used_ids_.insert(id);
}

auto fake_system_base::generate_object_id() -> obj_id_t
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

void fake_system_base::on_session_loaded() 
{
    session_loaded_signal_() ;
}

void fake_system_base::on_session_stopped()
{
    session_stopped_signal_() ;

    vector<obj_id_t> roots = remove_roots_order();

    for (auto it = roots.begin(); it != roots.end(); ++it)
        if (objects_.count(*it) != 0) // e.g. fpl_manager removes corresponding aircraft on fpl destroying 
            process_destroy_object(*it);

    Assert(root_objects_.empty());
}

void fake_system_base::on_time_factor_changed (double time, double factor)
{
    // it allows to restart atc_update after time changing (e.g. while moving slider in history)
    last_update_atc_ = size_t(time / atc_update_period()) * atc_update_period(); 
    time_factor_changed_signal_(time, factor);
}

object_info_ptr fake_system_base::create_object(object_class_ptr hier_class, std::string const &obj_name)
{
    object_info_ptr obj;
    msgs_blocker    mb(*this);
    FIXME(Доп функционал)
    {
        locks::bool_lock l(create_object_lock_);
        obj = create_object_hierarchy_impl(hier_class, obj_name, true);
    }

    if (obj)
        fire_object_created(obj);

    return obj;
}

object_info_ptr fake_system_base::create_object(obj_create_data const& data)
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

object_info_ptr fake_system_base::load_object_hierarchy(dict_t const& dict)
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

void fake_system_base::save_object_hierarchy(object_info_ptr objinfo, dict_t& dict, bool safe_key) const
{
    //     optional<time_counter> tc;
    //     if (objinfo->parent().expired())
    //         tc = in_place();

    write(dict, objinfo->hierarchy_class()->name(), "hierarchy_class_name");
    write(dict, objinfo->name()     , "name");
    write(dict, objinfo->object_id(), "id"  );

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

    auto fp = fn_reg::function<object_info_ptr(kernel::object_create_t const&)>(function_name);
    auto fp_d = fn_reg::function<object_info_ptr(kernel::object_create_t const&, dict_copt dict)>(function_name);

    if(fp)
        return fp(oc);
    else if(fp_d)
        return fp_d(oc,dict);
    else
        return fn_reg::function<object_info_ptr(kernel::object_create_t const&, dict_copt)>(/*lib_name,*/ class_name+"_view")(boost::cref(oc), dict);


    return object_info_ptr();
}


object_info_ptr fake_system_base::create_object_hierarchy_impl(
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

        auto msg_service = boost::bind(&fake_system_base::send_obj_message, this, id, _1, _2, _3);
        auto block_msgs  = [this](bool block){ block_obj_msgs(block); };

        kernel::object_create_t oc(hierarchy_class, this, id, name, children, msg_service, block_msgs);

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

object_info_ptr fake_system_base::load_object_hierarchy_impl(
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
    auto msg_service = boost::bind(&fake_system_base::send_obj_message, this, id, _1, _2, _3);
    auto block_msgs  = [this](bool block){ block_obj_msgs(block); };


    kernel::object_create_t oc(hierarchy_class, this, id, name, children, msg_service, block_msgs);

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

object_class_vector const& fake_system_base::object_classes() const
{
    return root_->classes();
}

object_class_ptr fake_system_base::get_object_class(std::string const& name) const
{
    return root_->find_class(name);
}

std::string fake_system_base::generate_unique_name(std::string const &init_name) const
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

void fake_system_base::fire_object_created(object_info_ptr obj)
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
void fake_system_base::on_object_created(msg::object_created const& msg)
{
    dict_t dict;
    binary::unwrap(msg.data, dict);

    load_object_hierarchy(dict);
}

void fake_system_base::on_object_destroy(msg::destroy_object const& msg)
{
    process_destroy_object(msg.obj_id);
}

void fake_system_base::on_object_msg(msg::object_msg const& msg)
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

void fake_system_base::on_container_msg(msg::container_msg const& msg)
{                                                      
    for (size_t i = 0; i < msg.msgs.size(); ++i)
        msg_disp_.dispatch_bytes(msg.msgs[i]);
}

void fake_system_base::do_pre_update(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
        base_presentation_ptr(it->second)->pre_update(time);
}

void fake_system_base::do_update(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
    {
        base_presentation_ptr(it->second)->update(time);

    }
}

void fake_system_base::do_post_update(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
        base_presentation_ptr(it->second)->post_update(time);
}

void fake_system_base::do_update_atc(double time)
{
    for (objects_t::iterator it = root_objects_.begin(); it != root_objects_.end(); ++it)
        base_presentation_ptr(it->second)->update_atc(time);
}

network::msg_dispatcher<>& fake_system_base::msg_disp()
{
    return msg_disp_;
}

void fake_system_base::block_obj_msgs(bool block)
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

void fake_system_base::send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd)
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

void fake_system_base::process_destroy_object( size_t object_id )
{
    auto it = objects_.find(object_id);
    if(it == objects_.end())
    {
        LogError("Destroying already destroyed object. Are you destroying object while playing history?");
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
vector<string> const& fake_system_base::auto_object_order()
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

vector<obj_id_t> fake_system_base::remove_roots_order()
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

void fake_system_base::check_destroy(std::vector<object_info_wptr> const& objs_to_destroy)
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
                LOG_ODS_MSG ("object (id = " << it->lock()->object_id() << ", name = " << name << ", sys = " << sys_name(kind()) << ") can't be destroyed  (use_count = " << use_count << ")");
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

bool fake_system_base::obj_msgs_blocked() const
{
    return block_obj_msgs_counter_ > 0;
}

struct model_system_impl
    : model_system
    , fake_system_base
{
    model_system_impl(msg_service& service, string const& script);

    // model_system
public:
    double calc_step() const override;

private:
    double time_factor_;
    double calc_step_;

private:
    // py_engine py_engine_; 
    string    ex_script_;

private:
    bool             skip_post_update_;
    optional<double> last_update_time_;
};

FIXME(Имя файла объектов в конструкторе это очень на круто)
model_system_impl::model_system_impl(msg_service& service, string const& script)
    : fake_system_base( sys_model, service, "objects.xml" )
    , time_factor_( 0)
    , calc_step_  (cfg().model_params.msys_step)
    //, py_engine_  (this)
    , ex_script_  (script)

    , skip_post_update_(true)
{
    LogInfo("Create Model Subsystem");
    // subscribe_session_stopped(boost::bind(&model_system_impl::on_ses_stopped, this));
}

double model_system_impl::calc_step() const
{
    return calc_step_ * (cg::eq_zero(time_factor_) ? 1. : time_factor_);
}


#pragma region visual system define



struct visual_system_impl
    : visual_system
    , visual_system_props
    , fake_system_base
    //, victory::widget_control
{

    visual_system_impl(msg_service& service, av::IVisualPtr vis, vis_sys_props const& props);

private:
    void update       (double time) override;
    void load_exercise(dict_cref dict) override;

    // visual_system
private:
#ifdef ASYNC_OBJECT_LOADING
    visual_object_ptr       create_visual_object( std::string const & res, uint32_t seed = 0 , bool async=true);
    visual_object_ptr       create_visual_object( nm::node_control_ptr parent, std::string const & res, uint32_t seed = 0, bool async=true );
#else
	visual_object_ptr       create_visual_object( std::string const & res, uint32_t seed = 0 , bool async=false);
	visual_object_ptr       create_visual_object( nm::node_control_ptr parent, std::string const & res, uint32_t seed = 0, bool async=false );
#endif

    // visual_system_props
private:

    vis_sys_props const&    vis_props   () const;
    void                    update_props(vis_sys_props const&);

private:
    void            init_eye  ();
    void            update_eye();
    cg::camera_f    eye_camera() const;
    void            object_destroying(object_info_ptr object);

private:
    av::IVisualPtr            visual  () override;
    av::IScenePtr             scene   () override;

private:
    DECL_LOGGER("vis_sys");

private:
    vis_sys_props           props_;

    av::IVisualPtr                     vis_;
    av::IScenePtr                    scene_;
    //victory::IViewportPtr   viewport_;

    scoped_connection object_destroying_connection_;

//private:
//    void init_frustum_projection();

private:
    visual_control_ptr      eye_;


};


visual_system_impl::visual_system_impl(msg_service& service, av::IVisualPtr vis, vis_sys_props const& props)
    : fake_system_base(sys_visual, service, "objects.xml")
    , vis_   (vis)
    , scene_ (vis->GetScene())
    //, viewport_ (vis->create_viewport())

    , props_(props)
    , object_destroying_connection_(this->subscribe_object_destroying(boost::bind(&visual_system_impl::object_destroying, this, _1)))
{
    LogInfo("Create Visual Subsystem");
}

av::IVisualPtr   visual_system_impl::visual()
{
    return  vis_;
}

av::IScenePtr    visual_system_impl::scene()
{
    return scene_;
}

void visual_system_impl::update(double time)
{
    fake_system_base::update(time);

    //scene_->update(time);
    //update_eye();
}

vis_sys_props const& visual_system_impl::vis_props() const
{
    return props_;
}

void visual_system_impl::update_props(vis_sys_props const& props)
{
    props_ = props;
    init_eye();
}

void visual_system_impl::load_exercise(dict_cref dict)
{
    fake_system_base::load_exercise(dict);
    init_eye();
}

visual_object_ptr visual_system_impl::create_visual_object( std::string const & res, uint32_t seed/* = 0*/, bool async )
{
    return boost::make_shared<visual_object_impl>( res, seed, async);
}

visual_object_ptr visual_system_impl::create_visual_object( nm::node_control_ptr parent,std::string const & res, uint32_t seed/* = 0*/, bool async )
{
    return boost::make_shared<visual_object_impl>( parent, res, seed, async);
}

void visual_system_impl::init_eye()
{
    // no eye is selected if camera name is incorrect to show problem visually!!!
    // please don't change selection logic
    eye_ = (!props_.channel.camera_name.empty()) 
        ? find_object<visual_control_ptr>(this, props_.channel.camera_name)
        : find_first_object<visual_control_ptr>(this) ;
#if 0
    if (!eye_ && !props_.channel.camera_name.empty())
        LogWarn("Can't find camera: " << props_.channel.camera_name);

    viewport_->SetClarityScale(props_.channel.pixel_scale);
    viewport_->set_geom_corr(props_.channel.cylindric_geom_corr ? victory::IViewport::explicit_cylinder : victory::IViewport::no_geom_corr);

    init_frustum_projection();
#endif
    update_eye();

}

void visual_system_impl::update_eye()
{
    const auto & cam = /*debug_eye_ ? debug_eye_camera() :*/ eye_camera();
    //viewport_->SetPosition(cam.position(), cam.orientation());
	vis_->SetPosition(cam.position(), cg::quaternion(cam.orientation()));
}

cg::camera_f visual_system_impl::eye_camera() const
{
    typedef cg::rotation_3f rot_f;

    cg::camera_f cam = eye_ 
        ? cg::camera_f(point_3f(geo_base_3(props_.base_point)(eye_->pos())), cprf(eye_->orien()))
        //: cg::camera_f();
		: cg::camera_f(point_3f(470,950,100), cprf(120,0,0));
	
    cam.set_orientation((rot_f(cam.orientation()) * rot_f(cprf(props_.channel.course))).cpr());
    return cam;
}

void visual_system_impl::object_destroying(object_info_ptr object)
{
    if (eye_ == object)
        eye_.reset();
    //else if (debug_eye_ && debug_eye_->track_object == object)
    //    debug_eye_.reset();
}
#pragma  endregion


#pragma region control system define

struct ctrl_system_impl
    : ctrl_system
    , fake_system_base

{

    ctrl_system_impl(msg_service& service);

private:
    void update       (double time) override;

private:


    scoped_connection object_destroying_connection_;



};


ctrl_system_impl::ctrl_system_impl(msg_service& service)
    : fake_system_base(sys_ext_ctrl, service, "objects.xml")
    //, object_destroying_connection_(this->subscribe_object_destroying(boost::bind(&ctrl_system_impl::object_destroying, this, _1)))
{
    LogInfo("Create Control Subsystem");
}

void ctrl_system_impl::update(double time)
{
    fake_system_base::update(time);
}

#pragma  endregion

} // kernel