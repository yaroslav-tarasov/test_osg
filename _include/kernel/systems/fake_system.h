#pragma once
  
#include "av/IVisual.h"

namespace kernel
{
struct vis_sys_props;

system_ptr create_model_system ( msg_service& service, std::string const& script);
system_ptr create_visual_system(msg_service& service, av::IVisualPtr vis, vis_sys_props const& vsp ); 
system_ptr create_ctrl_system  ( msg_service& service );

}