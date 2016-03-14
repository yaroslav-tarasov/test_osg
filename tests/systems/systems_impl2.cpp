
#include "stdafx.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

#include "kernel/systems.h"
#include "kernel/msg_proxy.h"

#include "factory_systems.h"
#include "systems_impl_common.h"
#include "systems_impl2.h"


namespace second
{

    impl::impl(remote_send_f rs)
        : systems(rs)
        , _msys(nullptr)
        , _csys(nullptr)
        , msg_service_    (boost::bind(&impl::push_back_all, this, _1))
    {

    }

    systems_ptr  impl::get_this()
    {
        return shared_from_this();
    }


    void impl::push_back_all (binary::bytes_cref bytes)
    {
        queue_.push_back(bytes);
        if(rs_) 
            rs_(bytes, true);
    }

    void impl::push_back (binary::bytes_cref bytes)
    {
        queue_.push_back(bytes);
    }

    void impl::update_messages()
    {
        while(queue_.size()>0)
        {
            if(queue_.front().size()>0)
                msg_service_.on_remote_recv(queue_.front(),true);
            queue_.pop_front();
        }
    }

    kernel::system_ptr impl::get_control_sys()
    { 
        if(!_csys)
            _csys = create_ctrl_system(msg_service_);
        return  _csys;
    }

    kernel::system_ptr impl::get_visual_sys(av::IVisualPtr)   
    {
        return  nullptr;
    }

    kernel::system_ptr impl::get_model_sys()    
    {
        if (!_msys)
            _msys = create_model_system(msg_service_, "place script here");
        return  _msys;
    }

    void impl::create_auto_objects()   
    {
        create_auto_object(_csys,"phys_sys","phys_sys");
        create_auto_object(_csys,"airports_manager","aiports_manager");
        create_auto_object(_csys,"ada","ada");
        create_auto_object(_csys,"meteo_proxy","meteo_proxy");
        create_auto_object(_csys,"aircraft_reg","aircraft_reg");
    }                                                             


}