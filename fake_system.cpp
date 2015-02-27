#include "stdafx.h"

#include "kernel/object_class.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems.h"
#include "kernel/kernel.h"

#include "fake_system.h"


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
struct model_system_impl;

system_ptr create_model_system(/*msg_service& service,*/ std::string const& script) 
{
    LogInfo("Creating MODEL system");
    return kernel::system_ptr(boost::make_shared<model_system_impl>(/*boost::ref(service),*/ boost::ref(script)));
}

struct  fake_system_base
    : system            // интерфейс
    //, system_session    // интерфейс
    //, objects_factory   // интерфейс
    , fake_objects_factory
    , object_collection // интерфейс
    , boost::enable_shared_from_this<fake_system_base>
{
    fake_system_base(system_kind kind/*, msg_service& service*/, std::string const &objects_file_name);
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

    // objects_factory
protected:
    object_info_ptr create_object       (object_class_ptr hierarchy_class, std::string const &name)     override;
    object_info_ptr create_object       (std::string const &object_name)                                override;
    object_class_vector const& object_classes() const                                                   override;
    object_class_ptr get_object_class(std::string const& name) const                                    override;

    // object_collection
protected:
    objects_t const&    root_objects    () const override;
    object_info_ptr     get_object      (obj_id_t object_id) const override;
    void                destroy_object  (obj_id_t object_id) override;

protected:
    void send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd);

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
    // object_info_ptr load_object_hierarchy_impl  (object_class_ptr parent_hierarchy_class, dict_cref dict, bool is_root, bool read_id);

protected:
    typedef ph_set<obj_id_t>::set_t obj_set_t;

protected:
    system_kind     kind_;
    // msg_service_reg msg_service_;

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
    struct msgs_blocker;
    void block_obj_msgs  (bool block);
};  

fake_system_base::fake_system_base(system_kind kind/*, msg_service& service*/, std::string const &objects_file_name )
    : kind_                 (kind)
    //, msg_service_          (service, this)
    //, create_object_lock_   (false)
    //, id_randgen_           (randgen_seed_tag())
    //, udp_messages_size_    (0)
    //, udp_msg_threshold_    (1300 ) // limit for Win32
    //, block_obj_msgs_counter_(0)
{
    // загружаем файл objects.xml - в нем дерево подсистем и объектов, в том числе ссылки на внешние файлы ani, fpl, bada
    tinyxml2::XMLDocument units_doc;
    //fs::path objects_xml = fs::path(cfg().path.data) / objects_file_name; 
    //Verify(is_regular_file(objects_xml));

    xinclude_load_file(units_doc, objects_file_name/*objects_xml.string()*/, 2);
    // строим дерево 
    root_ =     create_object_class(units_doc.RootElement());

    //msg_disp_
    //    .add<msg::object_created>(boost::bind(&system_base::on_object_created, this, _1))
    //    .add<msg::destroy_object>(boost::bind(&system_base::on_object_destroy, this, _1))
    //    .add<msg::object_msg    >(boost::bind(&system_base::on_object_msg    , this, _1))
    //    .add<msg::container_msg >(boost::bind(&system_base::on_container_msg , this, _1));

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

void fake_system_base::update (double /*time*/)
{
}
void fake_system_base::on_msg(binary::bytes_cref bytes)
{
    // msg_disp_.dispatch_bytes(bytes);
}

//! базовая функция загрузки упражнения
void fake_system_base::load_exercise(dict_cref dict)
{
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
    //msg_service_((system_kind)kind_, network::wrap_msg(msg::destroy_object(object_id)), true);
    //process_destroy_object(object_id);
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

object_info_ptr   fake_system_base::create_object  (std::string const& name)
{   
    object_info_ptr obj;
    //msgs_blocker    mb(*this);

    //{
    //    locks::bool_lock l(create_object_lock_);
    //    obj = load_object_hierarchy_impl(object_class_ptr(), data.dict(), true, false);
    //}

    uint32_t id       = generate_object_id();

    auto msg_service = boost::bind(&fake_system_base::send_obj_message, this, id, _1, _2, _3);
    auto block_msgs  = [this](bool block){ block_obj_msgs(block); };

    kernel::object_create_t oc(/*hierarchy_class*/nullptr, this, id, name, std::vector<object_info_ptr>(), msg_service, block_msgs);

    auto fp = fn_reg::function<object_info_ptr(kernel::object_create_t const&)>(name);
    
    if(fp)
        fp(oc);

    if (obj)
        fire_object_created(obj);
    
    if (obj == nullptr)
    {
        LogError("Unable to load object " << name);
        return obj;
    }

    register_obj_id(id);

    return obj;
}

object_info_ptr fake_system_base::create_object(object_class_ptr hier_class, std::string const &obj_name)
{
    object_info_ptr obj;
    //msgs_blocker    mb(*this);
    FIXME(Доп функционал)
    {
        //locks::bool_lock l(create_object_lock_);
        obj = create_object_hierarchy_impl(hier_class, obj_name, true);
    }

    if (obj)
        fire_object_created(obj);

    return obj;
}

FIXME(А это должно быть в отдельном файле)

inline object_info_ptr create_object(kernel::object_create_t oc)
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

    if(fp)
        return fp(oc);

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

object_class_vector const& fake_system_base::object_classes() const
{
    return root_->classes();
}

object_class_ptr fake_system_base::get_object_class(std::string const& name) const
{
    return root_->find_class(name);
}

void fake_system_base::fire_object_created(object_info_ptr obj)
{
    //send to other systems
    //dict_t dic;
    //save_object_hierarchy(obj, dic, false);

    //msg_service_((system_kind)kind_, network::wrap_msg(msg::object_created(binary::wrap(std::move(dic)))), true);

    //fire on my system
    object_created_signal_(obj);
}

void fake_system_base::block_obj_msgs(bool block)
{
    //if (block)
    //{
    //    if (block_obj_msgs_counter_ == 0)
    //        // this could happen via sending message, or via handling incoming one
    //        msg_protocol_ = in_place();

    //    ++block_obj_msgs_counter_;
    //}
    //else 
    //{
    //    Assert(block_obj_msgs_counter_ > 0);
    //    --block_obj_msgs_counter_;
    //}

}

void fake_system_base::send_obj_message(size_t object_id, binary::bytes_cref bytes, bool sure, bool just_cmd)
{
}

struct model_system_impl
    : model_system
    , fake_system_base
{
    model_system_impl(/*msg_service& service,*/ string const& script);

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
model_system_impl::model_system_impl(/*msg_service& service,*/ string const& script)
    : fake_system_base( sys_model/*, service*/, "objects.xml" )
    , time_factor_( 0)
    , calc_step_  (.1)
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

}