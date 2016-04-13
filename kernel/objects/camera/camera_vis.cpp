#include "stdafx.h"
#include "precompiled_objects.h"
#include "camera_vis.h"
#include "objects/nodes_management.h"

namespace camera_object
{

object_info_ptr vis::create(object_create_t const& oc, dict_copt dict)
{
    Verify(dict);
    return object_info_ptr(new vis(oc, dict));
}

vis::vis(object_create_t const& oc, dict_copt dict)
    : view          (oc, dict)
    , magnification_(1.)
{
}

void vis::update(double /*time*/)
{
#if 0
    if (binoculars_)
    {
        auto target_orien = rotation_3(vis::orien()) * rotation_3(binoculars().orien);

        if (track_obj_root_)
        {
            auto pp = geo_direction(pos(), track_obj_root_->position().global().pos);
            target_orien = rotation_3(cpr(pp.course, pp.pitch));
        }

        auto channel_orien = rotation_3(vis::orien()) * rotation_3(cpr(dynamic_cast<visual_system_props *>(sys_)->vis_props().channel.course, 0., 0.));
        auto loc_bin_orien = !channel_orien * target_orien;

        binoculars_->set_local_orient(loc_bin_orien);
        binoculars_->set_zoom_ratio  (binoculars().zoom);

    }
#endif
}

void vis::on_object_destroying(kernel::object_info_ptr object)
{
    if (track_obj_ == object)
    {
        track_obj_.reset() ;
        track_obj_root_.reset() ;
    }
}

geo_point_3 vis::pos() const 
{
    return view::pos(); 
}

cpr vis::orien() const 
{ 
    return view::orien(); 
}


//
void vis::on_new_binoculars()
{
    auto props = dynamic_cast<visual_system_props*>(sys_)->vis_props();

    if (props.channel.local)
        return;

    if (!props.channel.camera_name.empty() && props.channel.camera_name != name())
        return;
#if 0
    if (!binoculars().active)
    { 
        binoculars_ = 0;
        return;
    }


    if (binoculars().active && !binoculars_ && nodes_management::vis_node_control_ptr(root())->is_visible())
        binoculars_ = dynamic_cast<visual_system*>(sys_)->viewport()->create_binocular_view();
#endif

    track_obj_ = find_object<object_info_ptr>(collection_, binoculars().target) ;
    track_obj_root_ = utils::obj_root(track_obj_);

    if (!track_obj_root_)
        track_obj_.reset() ;
}

}

AUTO_REG_NAME(camera_visual, camera_object::vis::create);