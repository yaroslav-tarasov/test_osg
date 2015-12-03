#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_model.h"
#include "objects\common\aircraft_physless.h"


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
         aircraft_physless::info_ptr a = aircrafts_[msg.ext_id/*e2o_[msg.ext_id]*/];
         if(auto pa = aircraft_physless::model_control_ptr(a))
         {
             pa->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pa->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth ); 
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
	if (aircraft_physless::info_ptr airc_info = object)
		if(airc_info)
			add_aircraft(airc_info);
}

void model::on_object_destroying(object_info_ptr object)
{
	auto a = aircrafts_.find(object->object_id());
	if ( a != aircrafts_.end())
	{
		aircrafts_.erase(object->object_id());
	}
}

bool model::add_aircraft(aircraft_physless::info_ptr airc_info)
{
	size_t id = object_info_ptr(airc_info)->object_id();

	aircrafts_[id] = airc_info;

	return true;
}

} // end of aircraft_reg