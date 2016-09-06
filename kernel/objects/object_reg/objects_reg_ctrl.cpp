#include "stdafx.h"
#include "precompiled_objects.h"

#include "objects_reg_ctrl.h"

#include "objects/vehicle.h"
#include "objects/common/airport.h"
#include "objects/environment.h"
#include "objects/common/camera_common.h"
#include "objects/common/arresting_gear.h"

#include "common/ext_msgs.h"


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

    void (ctrl::*on_run)               (net_layer::msg::run_msg const& msg)           = &ctrl::inject_msg;
    void (ctrl::*on_container)         (net_layer::msg::container_msg    const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_atow)              (net_layer::msg::attach_tow_msg_t const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_dtow)              (net_layer::msg::detach_tow_msg_t const& msg)  = &ctrl::inject_msg;    
    void (ctrl::*on_malfunction)       (net_layer::msg::malfunction_msg  const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_engine_state)      (net_layer::msg::engine_state_msg const& msg)  = &ctrl::inject_msg;
    void (ctrl::*on_parachute_state)   (net_layer::msg::parachute_state_msg const& msg)    = &ctrl::inject_msg;
    void (ctrl::*on_fire)              (net_layer::msg::fire_fight_msg   const& msg)       = &ctrl::inject_msg;
    void (ctrl::*on_environment)       (net_layer::msg::environment_msg  const& msg)       = &ctrl::inject_msg;
    void (ctrl::*on_traj_assign)       (net_layer::msg::traj_assign_msg  const& msg)       = &ctrl::inject_msg;
	void (ctrl::*on_set_target)        (net_layer::msg::arrgear_target_msg  const& msg)    = &ctrl::inject_msg;
	void (ctrl::*on_destroy)           (net_layer::msg::destroy_msg  const& msg)           = &ctrl::inject_msg;
    void (ctrl::*on_create)            (net_layer::msg::create_msg  const& msg)            = &ctrl::inject_msg;
    void (ctrl::*on_update_cloud_zone) (net_layer::msg::update_cloud_zone_msg  const& msg) = &ctrl::inject_msg;

    

    disp_
        .add<net_layer::msg::run_msg               >(boost::bind(on_run         , this, _1))
        .add<net_layer::msg::container_msg         >(boost::bind(on_container   , this, _1))
        .add<net_layer::msg::attach_tow_msg_t      >(boost::bind(on_atow        , this, _1))
        .add<net_layer::msg::detach_tow_msg_t      >(boost::bind(on_dtow        , this, _1))
        .add<net_layer::msg::malfunction_msg       >(boost::bind(on_malfunction , this, _1))
        .add<net_layer::msg::engine_state_msg      >(boost::bind(on_engine_state, this, _1))
	    .add<net_layer::msg::parachute_state_msg   >(boost::bind(on_parachute_state, this, _1))
		.add<net_layer::msg::fire_fight_msg        >(boost::bind(on_fire        , this, _1))
        .add<net_layer::msg::traj_assign_msg       >(boost::bind(on_traj_assign , this, _1)) 
        .add<net_layer::msg::environment_msg       >(boost::bind(on_environment , this, _1))
        .add<net_layer::msg::arrgear_target_msg    >(boost::bind(on_set_target  , this, _1))
		.add<net_layer::msg::create_msg            >(boost::bind(on_create     , this, _1))	
        .add<net_layer::msg::destroy_msg           >(boost::bind(on_destroy     , this, _1))	
        .add<net_layer::msg::update_cloud_zone_msg >(boost::bind(on_update_cloud_zone     , this, _1))	
        ;

	create_object_f_ = fn_reg::function<kernel::object_info_ptr (kernel::system*, net_layer::msg::create_msg const&)>( "create_object");

}

void ctrl::on_object_created(object_info_ptr object)
{
	if(airport::info_ptr(object))
		airports_[object->object_id()]=object;

    auto data = object_data_ptr(object)->get_data();

    if (data>0)
    {
        e2o_[data] = object->object_id();
        regs_objects_[data] = object;
    }
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

void ctrl::inject_msg(net_layer::msg::create_msg const& msg)
{
    kernel::object_info_ptr  a = nullptr;


    if(create_object_f_)
        a = create_object_f_(sys_, msg);


    if (a)
    {
        e2o_[msg.ext_id] = a->object_id();
        regs_objects_[msg.ext_id] = a;
    }
}

void ctrl::inject_msg( net_layer::msg::destroy_msg const& msg)
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
 
void ctrl::inject_msg(net_layer::msg::update_cloud_zone_msg const& msg)
{
    // 
    auto a = regs_objects_.find(msg.ext_id);
    if ( a != regs_objects_.end())
    {

    }
    else
    {
        kernel::object_info_ptr  obj = nullptr;
        
        auto create_object_f = fn_reg::function<kernel::object_info_ptr (kernel::system*, net_layer::msg::update_cloud_zone_msg const&)>( "create_cloud_zone");
        if(create_object_f)
        {
             obj = create_object_f(sys_, msg);

            if (obj)
            {
                e2o_[msg.ext_id] = obj->object_id();
                regs_objects_[msg.ext_id] = obj;
            }
        }
    }

}

void ctrl::inject_msg(const void* data, size_t size)
{
    binary::bytes_t msg (bytes_raw_ptr(data), bytes_raw_ptr(data) + size);
    messages_.push_back(std::move(msg));
}

void ctrl::inject_msg(net_layer::msg::run_msg const& msg)
{
    net_layer::msg::run_msg  smsg = msg;
    smsg.ext_id = e2o_[msg.ext_id];
    set(smsg);
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


void ctrl::inject_msg(net_layer::msg::traj_assign_msg const& msg) 
{
    if(msg.ext_id>0 )                          
    {
        auto a = regs_objects_[msg.ext_id];

        if (camera_object::control_ptr pa = a)
        {
            pa->set_trajectory(msg.traj);
        }
        else if (aircraft::control_ptr pa = a)
        {
            set(net_layer::msg::traj_assign_msg( msg.ext_id, msg.traj));
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

        if (aircraft::control_ptr pa = a)
        {
            pa->set_equipment_state(aircraft::equipment_state_t((aircraft::engine_state_t)msg.state, aircraft::PS_SIZE));
        }
    }

}

void ctrl::inject_msg(net_layer::msg::parachute_state_msg const& msg) 
{
    if(msg.ext_id>0 )                          
    {
        auto a = regs_objects_[msg.ext_id];

        if (aircraft::control_ptr pa = a)
        {
            pa->set_equipment_state(aircraft::equipment_state_t(aircraft::ES_SIZE, (aircraft::parachute_state_t)msg.state));
        }
    }

}

 
void ctrl::inject_msg(net_layer::msg::attach_tow_msg_t  const& ext_id)  
{
    if(ext_id>0 )                          
    {
        auto a = regs_objects_[ext_id];

        if (vehicle::control_ptr pv = a)
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

        if (vehicle::control_ptr pv = a)
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

void ctrl::inject_msg(net_layer::msg::fire_fight_msg  const& ext_id)  
{
	if(ext_id>0 )                          
	{
		auto v = regs_objects_[ext_id];

		if (vehicle::control_ptr pv = v)
		{
			pv->fire_fight();
		}
	}
}
   
void ctrl::inject_msg(net_layer::msg::arrgear_target_msg const& ext_id)  
{
    if(ext_id>0 )                          
    {
        auto ap = regs_objects_[ext_id];
        
        if( ap && airports_.size()>0 )                          
        {
            for ( auto a = airports_.begin(); a != airports_.end(); ++a)
            {
                arresting_gear::control_ptr par = find_first_child<arresting_gear::control_ptr    >((*a).second);

                if (par)
                {
                    par->set_target(boost::optional<uint32_t>(ap->object_id()));
                    break;
                }
            }

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





void ctrl::set_sender(remote_send_f s)
{
     send_ = s;
}

void ctrl::pre_update(double time)
{
    base_view_presentation::pre_update(time);

    while(messages_.size()>0)    
    {
      if(messages_.front().size()>0)
		  disp_.dispatch_bytes(messages_.front());
	  
	  messages_.pop_front();
    }
}

} // end of objects_reg