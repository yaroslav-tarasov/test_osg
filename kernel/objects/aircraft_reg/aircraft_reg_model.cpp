#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_model.h"
//#include "objects\common\aircraft_physless.h"
#include "objects\common\vehicle.h"

namespace objects_reg
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
         
         if(auto pa = aircraft::model_ext_control_ptr(a))
         {
             auto it = last_msg_.find(msg.ext_id);
             if(it!=last_msg_.end() && msg.reverse != it->second.reverse)
             {
                 //pa->set_reverse(msg.reverse);
             }
             
             cg::cpr vc(msg.orien.cpr());
             pa->set_desired  (msg.time,msg.keypoint,msg.reverse?cg::cpr(cg::norm360(msg.orien.get_course() + msg.reverse * 180), msg.orien.get_pitch(), msg.orien.get_roll()):msg.orien,msg.speed);

             // pa->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pa->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth ); 
         }
         else if (vehicle::model_control_ptr pv = vehicle::model_control_ptr(a))
         {
             auto it = last_msg_.find(msg.ext_id);
             if(it!=last_msg_.end() && msg.reverse != it->second.reverse)
             {
                 //pv->set_reverse(msg.reverse);
             }

             //if(!msg.reverse)
                pv->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);

             pv->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth );         
         }
         
         
         last_msg_[msg.ext_id] =  msg;
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
	if (aircraft::info_ptr info = object)
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

} // end of objects_reg