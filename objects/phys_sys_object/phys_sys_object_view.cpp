#include "stdafx.h"
#include "precompiled_objects.h"

#include "phys_sys_object_view.h"

namespace phys
{
    object_info_ptr view::create(kernel::object_create_t const& oc/*, dict_copt dict*/)
    {
        return object_info_ptr(new view(oc/*, dict*/));
    }


    view::view( kernel::object_create_t const& oc/*, dict_copt dict*/ )
        : base_view_presentation(oc)
    {
    }

    AUTO_REG_NAME(phys_sys_view, view::create);
    //AUTO_REG_NAME(phys_sys_chart, base_chart_presentation<view>::create);


}
