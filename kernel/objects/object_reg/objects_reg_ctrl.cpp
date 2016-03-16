#include "stdafx.h"
#include "precompiled_objects.h"

#include "objects_reg_ctrl.h"

#include "objects/vehicle.h"
#include "objects/common/airport.h"
#include "objects/environment.h"

#include "common/test_msgs.h"


namespace objects_reg
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_ext_ctrl, ctrl::create);

ctrl::ctrl( kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
	, _sys(oc.sys)
{

    void (ctrl::*on_run)         (net_layer::msg::run const& msg)               = &ctrl::inject_msg;
    void (ctrl::*on_container)   (net_layer::msg::container_msg    const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_atow)        (net_layer::msg::attach_tow_msg_t const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_dtow)        (net_layer::msg::detach_tow_msg_t const& msg)  = &ctrl::inject_msg;    
    void (ctrl::*on_malfunction) (net_layer::msg::malfunction_msg  const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_engine_state)(net_layer::msg::engine_state_msg const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_fire)        (net_layer::msg::fire_fight_msg_t const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_environment) (net_layer::msg::environment_msg  const& msg)  = &ctrl::inject_msg;

    disp_
        .add<net_layer::msg::run                   >(boost::bind(on_run         , this, _1))
        .add<net_layer::msg::container_msg         >(boost::bind(on_container   , this, _1))
        .add<net_layer::msg::attach_tow_msg_t      >(boost::bind(on_atow        , this, _1))
        .add<net_layer::msg::detach_tow_msg_t      >(boost::bind(on_dtow        , this, _1))
        .add<net_layer::msg::malfunction_msg       >(boost::bind(on_malfunction , this, _1))
	    .add<net_layer::msg::engine_state_msg      >(boost::bind(on_engine_state, this, _1))
		.add<net_layer::msg::fire_fight_msg_t      >(boost::bind(on_fire        , this, _1))
        
        .add<net_layer::msg::environment_msg       >(boost::bind(on_environment , this, _1))
        	
        ;

	f_ = fn_reg::function<kernel::object_info_ptr (kernel::system*, net_layer::msg::create const&)>( "create_object");

}

void ctrl::on_object_created(object_info_ptr object)
{
	if(airport::info_ptr(object))
		airports_[object->object_id()]=object;
}

void ctrl::on_object_destroying(object_info_ptr object)
{    
    e2o_t::iterator it =  std::find_if(e2o_.begin(),e2o_.end(),[&](const e2o_value_t& vt) { return vt.second == object->object_id(); });
    
    if(it != e2o_.end())
    {
        auto a = regs_objects_.find(it->second);
        if ( a != regs_objects_.end())
        {
            a->second.reset();
            regs_objects_.erase(a);
        } else
        {
            auto a = airports_.find(it->second);
            if ( a != airports_.end())
            {
                a->second.reset();
                airports_.erase(a);
            }
        }
    }
}

void ctrl::destroy_object( net_layer::msg::destroy_msg const& msg)
{
    auto a = regs_objects_.find(msg);
	if ( a != regs_objects_.end())
    {  
        size_t oid = a->second->object_id();
        a->second.reset();
        regs_objects_.erase(a);
		dynamic_cast<kernel::object_collection*>(_sys)->destroy_object(oid) ;
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
    auto const& msgs = msg.msgs; 
    for(auto it = msgs.begin(); it!= msgs.end(); ++it)    
    {
        disp_.dispatch_bytes(*it);
    }
}

void ctrl::inject_msg(net_layer::msg::malfunction_msg const& msg) 
{
	if(msg.ext_id>0 )                          
	{
		auto a = regs_objects_[msg.ext_id];

		if (aircraft::aircraft_ipo_control_ptr pa = aircraft::aircraft_ipo_control_ptr (a))
		{
			pa->set_malfunction((aircraft::malfunction_kind_t)msg.kind,msg.enabled);
		}
	}

}

void ctrl::inject_msg(net_layer::msg::environment_msg const& msg) 
{
    if( airports_.size()>0 )                          
    {
        auto a = airports_.begin()->second;

        environment::control_ptr ec = find_first_child<environment::control_ptr    >(a);

        if (ec)
        {
            ec->set_weather(msg.weather);
        }
    }

}

void ctrl::inject_msg(net_layer::msg::engine_state_msg const& msg) 
{
    if(msg.ext_id>0 )                          
    {
        auto a = regs_objects_[msg.ext_id];

        if (aircraft::control_ptr pa = aircraft::control_ptr (a))
        {
            pa->set_engine_state((aircraft::engine_state_t)msg.state);
        }
    }

}

 
void ctrl::inject_msg(net_layer::msg::attach_tow_msg_t  const& ext_id)  
{
    if(ext_id>0 )                          
    {
        auto a = regs_objects_[ext_id];

        if (vehicle::control_ptr pv = vehicle::control_ptr(a))
        {
            pv->attach_tow();
            conn_holder_ << pv->subscribe_detach_tow(boost::bind(&ctrl::on_detach_tow,this,ext_id, _1));
        }
    }
}

void ctrl::inject_msg(net_layer::msg::detach_tow_msg_t  const& ext_id)  
{
    if(ext_id>0 )                          
    {
        auto a = regs_objects_[ext_id];

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

void ctrl::inject_msg(net_layer::msg::fire_fight_msg_t  const& ext_id)  
{
	if(ext_id>0 )                          
	{
		auto v = regs_objects_[ext_id];

		if (vehicle::control_ptr pv = vehicle::control_ptr(v))
		{
			pv->fire_fight();
		}
	}
}

void ctrl::on_detach_tow (uint32_t ext_id, decart_position const& pos)
{
    
    if(send_)
        send_(network::wrap_msg(std::move(net_layer::msg::detach_tow_coords_msg_t(ext_id,pos.pos,pos.orien.get_course()))));

    LogInfo( 
        "Tow tractor detach pos= " << pos.pos.x << " y " << pos.pos.y << " h " << pos.pos.z  << " course " << pos.orien.get_course()  << "/n" 
        );
}



void ctrl::create_object(net_layer::msg::create const& msg)
{
	kernel::object_info_ptr  a = nullptr;

	if(f_)
		a = f_(sys_, msg);

	if (a)
    {
		e2o_[msg.ext_id] = a->object_id();
        regs_objects_[msg.ext_id] = a;
    }
}

void ctrl::set_sender(remote_send_f s)
{
     send_ = s;
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

} // end of objects_reg