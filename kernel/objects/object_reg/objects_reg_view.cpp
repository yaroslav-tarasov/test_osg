#include "stdafx.h"
#include "precompiled_objects.h"

#include "objects_reg_view.h"


namespace objects_reg
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_view, view::create);

view::view( kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{
}



} // end of objects_reg