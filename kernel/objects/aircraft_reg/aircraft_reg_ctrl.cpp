#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_ctrl.h"

//#include "objects\aircraft_physless.h"
#include "objects\vehicle.h"


namespace aircraft_reg
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_ext_ctrl, ctrl::create);

ctrl::ctrl( kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
{

    void (ctrl::*on_run)         (net_layer::msg::run const& msg)               = &ctrl::inject_msg;
    void (ctrl::*on_container)   (net_layer::msg::container_msg const& msg)     = &ctrl::inject_msg;
    void (ctrl::*on_atow)        (net_layer::msg::attach_tow_msg_t const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_dtow)        (net_layer::msg::detach_tow_msg_t const& msg)  = &ctrl::inject_msg;    
    void (ctrl::*on_malfunction) (net_layer::msg::malfunction_msg const& msg)   = &ctrl::inject_msg;

    disp_
        .add<net_layer::msg::run                   >(boost::bind(on_run         , this, _1))
        .add<net_layer::msg::container_msg         >(boost::bind(on_container   , this, _1))
        .add<net_layer::msg::attach_tow_msg_t      >(boost::bind(on_atow         , this, _1))
        .add<net_layer::msg::detach_tow_msg_t      >(boost::bind(on_dtow         , this, _1))
        .add<net_layer::msg::malfunction_msg       >(boost::bind(on_malfunction , this, _1))
        ;
}

void ctrl::on_object_created(object_info_ptr object)
{
}

void ctrl::on_object_destroying(object_info_ptr object)
{    
    auto a = objects_.find(object->object_id());
    if ( a != objects_.end())
    {
        objects_.erase(object->object_id());
    }
}

void ctrl::inject_msg(const void* data, size_t size)
{
    // disp_.dispatch(data, size);
    binary::bytes_t msg (bytes_raw_ptr(data), bytes_raw_ptr(data) + size);
    messages_.push_back(std::move(msg));
}

void ctrl::inject_msg(net_layer::msg::run const& msg)
{
#if 0
    buffer_.push_back(msg);
#else
    net_layer::msg::run  smsg = msg;
    smsg.ext_id = e2o_[msg.ext_id];
    set(smsg);
#endif

}

void ctrl::inject_msg( net_layer::msg::container_msg const& msg)
{
}

void ctrl::inject_msg(net_layer::msg::attach_tow_msg_t  const& ext_id)  
{
    if(ext_id>0 )                          
    {
        auto a = objects_[ext_id];

        if (vehicle::control_ptr pv = vehicle::control_ptr(a))
        {
            pv->attach_tow();
        }
    }
}

void ctrl::inject_msg(net_layer::msg::detach_tow_msg_t  const& ext_id)  
{
    if(ext_id>0 )                          
    {
        auto a = objects_[ext_id];

        if (vehicle::control_ptr pv = vehicle::control_ptr(a))
        {
            pv->detach_tow();
            cg::point_3 cur_pos = pv->get_local_position().pos;
            cg::point_3 d_pos   = pv->get_local_position().dpos;
            
            LogInfo( 
                "Tow tractor detach pos= " << cur_pos.x << " " << cur_pos.y << " " << cur_pos.z  << "/n" 
                );

        }
    }
}

void ctrl::inject_msg(net_layer::msg::malfunction_msg const& msg) 
{
}

void ctrl::create_object(net_layer::msg::create const& msg)
{
	auto fp = fn_reg::function<kernel::object_info_ptr (net_layer::msg::create const&)>("create_object");
	kernel::object_info_ptr  a = nullptr;

	if(fp)
		a = fp(msg);

	if (a)
    {
		e2o_[msg.ext_id] = a->object_id();
        objects_[msg.ext_id] = a;
    }
}

void ctrl::pre_update(double time)
{
    base_view_presentation::pre_update(time);

#if 0
    while(buffer_.size()>0)
    {
        net_layer::msg::run & msg = buffer_.front();
        msg.ext_id = e2o_[msg.ext_id];
		set(msg);
        buffer_.pop_front();
    }
#else 
    while(messages_.size()>0)    
    {
      disp_.dispatch_bytes(messages_.front());
      messages_.pop_front();
    }
#endif
}

} // end of aircraft_reg