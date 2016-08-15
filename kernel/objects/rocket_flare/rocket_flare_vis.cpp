#include "stdafx.h"
#include "precompiled_objects.h"

#include "rocket_flare_vis.h"
#include "common/fire_trace.h"



namespace rocket_flare
{

object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}
AUTO_REG_NAME(rocket_flare_visual, visual::create);

visual::visual(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
    , vsys_     (dynamic_cast<visual_system*>(sys_))  
{
    fs_ = boost::make_shared<visual_objects::fire_trace_support>(
        vsys_->create_visual_object(nm::node_control_ptr(root()),"sfx//rocket_flare.scg",0,0,false),
        root(), root(), transform_4() ); 

}


void visual::update( double time )
{   
    view::update(time);

    geo_base_3 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point;
    
    if (fs_)
    {
        fs_->set_update_time(time);
        
        fs_->update(time, point_3f(), base);
    }



}

void visual::on_new_contact_effect(double time, std::vector<contact_t> const& contacts)
{
    visual_system* vsys = dynamic_cast<visual_system*>(sys_);
    
    if(fs_)
        fs_.reset();

#if 0
    for (size_t i = 0; i < contacts.size(); ++i)
    {
        point_3f offset = contacts[i].offset;

        optional<size_t> closest_spark;
        for (size_t j = 0; j < sparks_.size(); ++j)
            if (!closest_spark || cg::distance(sparks_[j].contact.offset, offset) < cg::distance(sparks_[*closest_spark].contact.offset, offset))
                closest_spark = j;

        if (!closest_spark || cg::distance(sparks_[*closest_spark].contact.offset, offset) > 1.5)
            sparks_.push_back(
            sparks_t(
            vsys->create_visual_object("sfx//sparks.scg"),
            vsys->create_visual_object("sfx//friction_dust.scg"),
            contacts[i], time));
        else
        {   
            sparks_[*closest_spark].time    = time;
            sparks_[*closest_spark].contact = contacts[i];
        }
    }
#endif


}

void visual::on_new_state()
{   

}


} // rocket_flare 


