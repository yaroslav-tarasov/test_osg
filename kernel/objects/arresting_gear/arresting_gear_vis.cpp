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

    if (!ropes_object_ )
    {
        visual_system* vsys = dynamic_cast<visual_system*>(sys_);

        ropes_object_ = vsys->create_visual_object("arrested_gear.scg",0,false);
        ropes_weak_ptr_ = nullptr;
        if (auto ropes_node = findFirstNode(ropes_object_->node().get(),"RopesNode"))
        {
            ropes_weak_ptr_ = dynamic_cast<RopesNodePtr>(ropes_node);
        }
    }
    else
    {
         ropes_weak_ptr_->updateRopes(ropes_state());
    }

}


void visual::on_new_ropes_state()
{   

}


} // arresting_gear 


