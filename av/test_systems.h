#pragma once

#include "kernel/systems_fwd.h"

struct creator
{
    virtual ~creator(){};
    virtual kernel::system_ptr get_control_sys() =0;
    virtual kernel::system_ptr get_visual_sys()  =0;
    virtual kernel::system_ptr get_model_sys()   =0;
    virtual void create_auto_objects()           =0; 
};

typedef shared_ptr<creator>  creator_ptr;

creator_ptr sys_creator();