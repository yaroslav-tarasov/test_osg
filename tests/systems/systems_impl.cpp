
#include "stdafx.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

#include "kernel/systems.h"
#include "kernel/msg_proxy.h"

#include "factory_systems.h"
#include "systems_impl_common.h"
#include "systems_impl.h"


namespace first
{

    impl::impl(remote_send_f rs)
        : systems(rs)
        , _msys(nullptr)
        , _csys(nullptr)
        , _vsys(nullptr)
        , msg_service_    (boost::bind(&impl::push_back_all, this, _1))
        , msg_service_vis_(boost::bind(&impl::push_back, this, _1))
    {

    }

    systems_ptr  impl::get_this()
    {
        return shared_from_this();
    }


    void impl::push_back_all (binary::bytes_cref bytes)
    {
        queue_.push_back(bytes);
        queue_vis_.push_back(bytes);
    }

    void impl::push_back (binary::bytes_cref bytes)
    {
        queue_.push_back(bytes);
    }

    void impl::update_messages()
    {
        time_measure_helper_t th("impl::update_messages: ", [=](double t)->bool{return true; }); 
        while(queue_.size()>0)
        {
            if(queue_.front().size()>0)
                msg_service_.on_remote_recv(queue_.front(),true);
            queue_.pop_front();
        }
    }

    FIXME("Эту хренотень надо сделать частью сервиса")
    void impl::update_vis_messages()
    {
        while(queue_vis_.size()>0)
        {
            if(queue_vis_.front().size()>0)
                msg_service_vis_.on_remote_recv(queue_vis_.front(),true);

            queue_vis_.pop_front();
        }
    }

    kernel::system_ptr impl::get_control_sys()
    { 
        if(!_csys)
            _csys = create_ctrl_system(msg_service_);
        return  _csys;
    }

    kernel::system_ptr impl::get_visual_sys(av::IVisualPtr vis)   
    {
        kernel::vis_sys_props props_;
        props_.base_point = ::get_base();

        FIXME(damn properties)
            if (!_vsys)
                _vsys = create_visual_system(msg_service_vis_, vis, props_);
        return  _vsys;
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
        create_auto_object(_csys,"airports_manager","airports_manager");
        create_auto_object(_csys,"ada","ada");
        create_auto_object(_csys,"meteo_proxy","meteo_proxy");
        create_auto_object(_csys,"labels_manager","labels_manager");
        create_auto_object(_csys,"aircraft_reg","aircraft_reg");
        create_auto_object(_csys,"mdd","mdd");
    }                                                             


}