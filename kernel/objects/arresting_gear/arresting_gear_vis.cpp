#include "stdafx.h"
#include "precompiled_objects.h"

#include "arresting_gear_vis.h"

namespace arresting_gear
{

object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}
AUTO_REG_NAME(arresting_gear_visual, visual::create);

visual::visual(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
{
    

}


void visual::update( double time )
{   
    view::update(time);
}



} // arresting_gear 


