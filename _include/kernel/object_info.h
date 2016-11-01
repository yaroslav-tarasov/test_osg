#pragma once
#include "kernel_fwd.h"
#include "object_info_fwd.h"
#include "reflection/proc/dict.h"
#include "common/event.h"

namespace boost { namespace python { class object; }}

namespace kernel
{

//! интерфейс предоставления абстрактной информации об объекте внутри дерева объектов
struct object_info
{
    virtual string const&               name            () const = 0;
    virtual uint32_t                    object_id       () const = 0;
    virtual object_info_wptr            parent          () const = 0;
    virtual object_info_vector const&   objects         () const = 0;
    virtual object_class_ptr            hierarchy_class () const = 0;
    virtual void                        save            (dict_ref dict, bool key_safe) const = 0;
    virtual boost::python::object       py_ptr          () const = 0;       

    DECLARE_EVENT(name_changed, ()) ;

    virtual ~object_info(){}
};

//! возвратить первый дочерний объект
template<class T>
    T find_first_child(object_info const* obj)
{
    for (auto it = obj->objects().begin(); it != obj->objects().end(); ++it)
        if (T ptr = *it)
            return ptr;

    return T();
}

//! возвратить первый дочерний объект
template<class T>
    T find_first_child(object_info_ptr obj)
{
    for (auto it = obj->objects().begin(); it != obj->objects().end(); ++it)
        if (T ptr = *it)
            return ptr;

    return T();
}

//! посетить все объекты и выполнить для каждого какое-то действие
template<class F>
    void visit_all_hierarchy( object_info_ptr obj, F f )
    {
        for (auto it = obj->objects().begin(); it != obj->objects().end(); ++it)
            visit_all_hierarchy(*it, f);
        f(obj);
    }


} // kernel
