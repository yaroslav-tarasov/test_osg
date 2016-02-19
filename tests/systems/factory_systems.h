#pragma once

#include "kernel/systems_fwd.h"

struct systems;

typedef shared_ptr<systems>  systems_ptr;

struct systems
{
    typedef function<void(binary::bytes_cref /*bytes*/, bool /*sure*/)> remote_send_f;
    
    systems (remote_send_f rs) : rs_(rs) {}

    enum type {FIRST_IMPL, SECOND_IMPL};
    virtual ~systems(){};
    virtual kernel::system_ptr get_control_sys()      = 0;
    virtual kernel::system_ptr get_visual_sys()       = 0;
    virtual kernel::system_ptr get_model_sys()        = 0;
    virtual void               create_auto_objects()  = 0;
    virtual void               update_messages()      = 0;
    virtual void               update_vis_messages()  = 0;
    virtual systems_ptr        get_this()             = 0;

protected:
   
    remote_send_f                                      rs_;
};

systems_ptr get_systems(systems::type  type = systems::FIRST_IMPL, systems::remote_send_f rs = systems::remote_send_f());