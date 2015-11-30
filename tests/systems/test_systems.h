#pragma once

#include "kernel/systems_fwd.h"

struct systems
{
    virtual ~systems(){};
    virtual kernel::system_ptr get_control_sys() =0;
    virtual kernel::system_ptr get_visual_sys()  =0;
    virtual kernel::system_ptr get_model_sys()   =0;
    virtual void               create_auto_objects()   =0;
    virtual void               update_messages() =0;
    virtual void               update_vis_messages() =0;
};

typedef shared_ptr<systems>  systems_ptr;

systems_ptr get_systems();