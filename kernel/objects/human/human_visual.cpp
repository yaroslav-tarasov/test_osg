#include "stdafx.h"
#include "precompiled_objects.h"

#include "human_visual.h"


namespace human
{

object_info_ptr visual::create(object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}

AUTO_REG_NAME(human_visual, visual::create);

visual::visual(object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
{
}

void visual::update(double time)
{
    view::update(time);
    

}


} // human
