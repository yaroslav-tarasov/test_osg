#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_ctrl.h"


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
}

void ctrl::on_object_created(object_info_ptr object)
{
}

void ctrl::on_object_destroying(object_info_ptr object)
{
}


void ctrl::inject_msg(net_layer::msg::run const& msg)
{
    buffer_.push_back(msg);
    // messages_.push_back(network::wrap_msg(msg));
    // set(msg);
}

void ctrl::inject_msg( net_layer::msg::container_msg const& msg)
{
    //buffer_.push_back(msg);
}

void ctrl::create_object(net_layer::msg::create const& msg)
{
	auto fp = fn_reg::function<kernel::object_info_ptr (net_layer::msg::create const&)>("create_object");
	kernel::object_info_ptr  a = nullptr;

	if(fp)
		a = fp(msg);

	if (a)
		e2o_[msg.ext_id] = a->object_id();
}

void ctrl::pre_update(double time)
{
    base_view_presentation::pre_update(time);
    while(buffer_.size()>0)
    {
        net_layer::msg::run & msg = buffer_.front();
        msg.ext_id = e2o_[msg.ext_id];
		set(msg);
        buffer_.pop_front();
    }
}

} // end of aircraft_reg