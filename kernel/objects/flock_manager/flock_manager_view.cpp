#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_manager_view.h"

namespace flock
{

namespace manager
{


object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}


AUTO_REG_NAME(flock_manager_view, view::create);

view::view( kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
	, obj_data_base         (dict)
{
}

void view::on_object_created(object_info_ptr object)
{
    base_view_presentation::on_object_created(object);

    if (auto air = child::info_ptr(object))
    {
        roamers_.insert(air);
    }
}

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object) ;

    if (auto air = child::info_ptr(object))
    {
        roamers_.erase(air);
    }
}


} // manager

} // flock