#include "stdafx.h"
#include "precompiled_objects.h"

#include "rocket_flare_vis.h"



namespace rocket_flare
{

object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}
AUTO_REG_NAME(rocket_flare_visual, visual::create);

visual::visual(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
{
    

}


void visual::update( double time )
{   
    view::update(time);

#if 0
    if (!flare_object_ )
    {
        visual_system* vsys = dynamic_cast<visual_system*>(sys_);

        flare_object_ = vsys->create_visual_object("arrested_gear.scg",0,0,false);
        flare_weak_ptr_ = nullptr;
        if (auto flare_node = findFirstNode(flare_object_->node().get(),"RopesNode"))
        {
            flare_weak_ptr_ = dynamic_cast<RocketFlareNodePtr>(flare_node);
        }
    }
    else
    {
         flare_weak_ptr_->update(time);
    }
#endif

}


void visual::on_new_state()
{   

}


} // rocket_flare 


