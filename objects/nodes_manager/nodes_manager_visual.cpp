#include "stdafx.h"
#include "precompiled_objects.h"

#include "nodes_manager_visual.h"
#include "vis_node_impl.h"


namespace nodes_management
{

object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    visual * obj = new visual(oc, dict);
    object_info_ptr info(obj);
    obj->init();
    return info;
}

AUTO_REG_NAME(nodes_manager_visual, visual::create);

visual::visual( kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
    , sys_(dynamic_cast<visual_system *>(oc.sys))
{
}

void visual::init()
{
    view::init();
    apply_vis_model();
}

void visual::apply_model(string const& model)
{
    view::apply_model(model);
    apply_vis_model();
}

void visual::apply_vis_model()
{
    if (!settings_.model.empty())
        visual_object_ = sys_->create_visual_object(settings_.model /*+ "//" + settings_.model + ".scg"*/, object_id()) ;
    else 
        visual_object_.reset();

    for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
        vis_node_impl_ptr(*it)->on_visual_object_created();

}

} // nodes_management
