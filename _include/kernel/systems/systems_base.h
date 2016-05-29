#pragma once

#include "kernel/object_info.h"
#include "kernel/systems_fwd.h"
#include "reflection/proc/binary.h"
#include "reflection/proc/dict.h"
#include "common/event.h"
#include "common/util.h"

#include "bin_data/bin_data.h"
#include "bin_data/io_streams_fwd.h"
#include "alloc/pool_stl.h"
namespace kernel
{

typedef uint32_t            obj_id_t;
typedef optional<obj_id_t>  obj_id_opt;

//! список подсистем тренажера
SYSTEMS_API enum system_kind
{
    sys_chart       ,
    sys_model       ,
    sys_visual      ,
    sys_sound       ,
    sys_atc_gate    , // aka sys_worm :)
    sys_history     ,
    sys_vcs         ,
    sys_dman_gate   ,
    sys_ext_ctrl    ,
};

//! таблица имен подсистем
inline std::string sys_name(system_kind kind)
{
    static std::string name[] = 
    {
        "chart"     , 
        "model"     , 
        "visual"    , 
        "sound"     , 
        "atc_gate"  , 
        "history"   ,
        "vcs"       ,
        "dman_gate" ,
        "ext_ctrl"
    };

    Assert((size_t)kind < util::array_size(name));
    return name[kind];
}



//! интерфейс системной сессии (??)
struct system_session
{
    virtual ~system_session(){}

    virtual void on_session_loaded      () = 0;
    virtual void on_session_stopped     () = 0;
    virtual void on_time_factor_changed (double time, double factor) = 0;

    //! сигнал Сессия загружена
    DECLARE_EVENT(session_loaded    ,   ());
    //! сигнал Сессия остановлена
    DECLARE_EVENT(session_stopped   ,   ());
    //! сигнал Изменен коэффициент хода времени
    DECLARE_EVENT(time_factor_changed,  (double /*time*/, double /*factor*/));
};

//! некая иерархическая структура данных (??) как-то использующаяся для создания объектов
//! через нее передается информация об иерархии объектов в фабрику объектов??
struct SYSTEMS_API obj_create_data
{
    obj_create_data(std::string const& hier_class, std::string const& name, dict_t&& own_data);
    obj_create_data(std::string const& hier_class, std::string const& name);

    obj_create_data&    add_child(obj_create_data const&);
    dict_t const&       dict     () const;

private:
    dict_t  data_;
    dict_t& children_;
};

//! интерфейс фабрики объектов - создание объектов, информация об объектах и прочее
struct objects_factory
{
    virtual ~objects_factory(){}

    virtual object_info_ptr create_object        (object_class_ptr hier_class, std::string const &name) = 0;
    virtual object_info_ptr create_object        (obj_create_data const& descr)                         = 0;

    virtual object_info_ptr load_object_hierarchy(dict_t const& dict)                                         = 0;
    virtual void            save_object_hierarchy(object_info_ptr objinfo, dict_t& dict, bool safe_key) const = 0;

    virtual object_class_vector const&  object_classes         ()                        const = 0;
    virtual object_class_ptr            get_object_class       (std::string const& name) const = 0;

    virtual std::string                 generate_unique_name   (std::string const &init_name) const = 0;
};

//! интерфейс коллекции объектов (??)
struct object_collection
{
    virtual ~object_collection(){}

    typedef ph_map<size_t, object_info_ptr>::map_t objects_t;

    virtual objects_t const&    root_objects  ()                   const = 0;
    virtual object_info_ptr     get_object    (obj_id_t object_id) const = 0;
    virtual void                destroy_object(obj_id_t object_id)       = 0 ;

    DECLARE_EVENT(object_created   , (object_info_ptr));
    DECLARE_EVENT(object_destroying, (object_info_ptr));
};

//! интерфейс объекта дерева (??) добавление и удаление дочерних объектов
struct tree_object
{
    virtual ~tree_object(){}

    virtual void append_child( object_info_ptr object ) = 0;
    virtual void remove_child( object_info_ptr object ) = 0;

    DECLARE_EVENT(child_appended, (object_info_ptr));
    DECLARE_EVENT(child_removing, (object_info_ptr));
    DECLARE_EVENT(parent_changed, ());
};

//! интерфейс любого вида/представления; универсальные функции обновления
struct base_presentation
    : object_info   // интерфейс информации об объекте внутри дерева
    , tree_object   // интерфейс добавления и удаления дочерних объектов в дерево
{
    virtual ~base_presentation(){}

    // always used ONLY(!) by view presentation 
    virtual void pre_update (double time)                               = 0; 

    // usual update by domain specific presentation (mod, vis, crt)
    virtual void update     (double time)                               = 0;

    // commonly used by group-management objects (physics, ai, collision, ...)
    virtual void post_update(double time)                               = 0;

    // update used by arm atc for drawing ratio
    virtual void update_atc(double time) = 0;


    virtual void on_msg     (binary::bytes_cref bytes)  = 0;
    virtual void reset_parent (object_info_wptr parent = object_info_wptr())   = 0;
};


typedef std::list<obj_create_data> creating_objects_list_t;

//! интерфейс системы - универсальный интерфейс общий для всех подсистем
struct system
{
	virtual ~system(){}

	virtual system_kind         kind    () const                    = 0;

	virtual void                update  (double time)               = 0;
	virtual void                on_msg  (binary::bytes_cref bytes)  = 0;

	virtual void                load_exercise(dict_cref dict)       = 0;
	virtual void                save_exercise(dict_ref  dict, bool safe_key) const = 0;

	virtual void                pack_exercise(const creating_objects_list_t& co , dict_ref dict, bool safe_key) /*const*/= 0;

	virtual optional<double>    update_time     () const            = 0;
	virtual optional<double>    last_update_time() const            = 0;

	virtual double              atc_update_period() const = 0;

	DECLARE_EVENT(exercise_loaded, ());
};

// used by any presentation to send message through its system,
typedef
    boost::function<void(binary::bytes_cref/*msg*/, bool /*sure*/, bool /*just_cmd*/)>
    send_msg_f;

typedef 
    boost::function<void(bool /*block*/)>
    block_obj_msgs_f;

//! возвратить первый объект коллекции
template<class T>
    T find_first_object(object_collection * collection)
{
    for (auto it = collection->root_objects().begin(); it != collection->root_objects().end(); ++it)
        if (T ptr = it->second)
            return ptr;

    return T();
}

//! найти объект в коллекции по имени
template<class T>
    T find_object(object_collection const* collection, string const& name)
{
    for (auto it = collection->root_objects().begin(); it != collection->root_objects().end(); ++it)
        if (it->second->name() == name)
            if (T ptr = it->second)
                return ptr;

    return T();
}

//! посетить все объекты в коллекции и выполнить для каждого какое-то действие
template<class T, class func>
    void visit_objects(object_collection const* collection, func const& f)
{
    for (auto it = collection->root_objects().begin(); it != collection->root_objects().end(); ++it)
    {
        if (T ptr = it->second)
            if (!f(ptr))
                return;
    }
}

} // kernel
