#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_model.h"
#include "objects\common\aircraft_physless.h"
#include "objects\common\vehicle.h"

namespace aircraft_reg
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict)
    : view                 (oc, dict)
{
    void (model::*on_run)  (net_layer::msg::run const& msg)             = &model::on_inject_msg;
    void (model::*on_malf) (net_layer::msg::malfunction_msg const& msg) = &model::on_inject_msg;

    msg_disp()
        .add<net_layer::msg::run >(boost::bind(on_run      , this, _1))
        .add<net_layer::msg::malfunction_msg >(boost::bind(on_malf      , this, _1))
        ;
}

void model::on_inject_msg(net_layer::msg::run const& msg)
{
     if(msg.ext_id>0 )                          
     {
         auto a = objects_[msg.ext_id];

         if(auto pa = aircraft_physless::model_control_ptr(a))
         {
             pa->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pa->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth ); 
         }
         else if (vehicle::model_control_ptr pv = vehicle::model_control_ptr(a))
         {
             pv->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pv->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth ); 
         }
     }
}

void model::on_inject_msg(net_layer::msg::malfunction_msg const& msg)
{

}

void model::on_inject_msg(net_layer::msg::container_msg const& msg)
{

}    


void model::on_object_created(object_info_ptr object)
{
	if (aircraft_physless::info_ptr info = object)
	{
        if(info)
			add_object(info);
    }
    else if (vehicle::model_info_ptr info = object)
    {
        if(info)
            add_object(info);
    }

}

void model::on_object_destroying(object_info_ptr object)
{
	auto a = objects_.find(object->object_id());
	if ( a != objects_.end())
	{
		objects_.erase(object->object_id());
	}
}

bool model::add_object(object_info_ptr object)
{
	size_t id = object->object_id();

	objects_[id] = object;

	return true;
}

} // end of aircraft_reg