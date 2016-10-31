#include "objects_reg_model.h"
//#include "common/aircraft_physless.h"
#include "common/vehicle.h"
#include "flock_manager/flock_manager_common.h"
#include "common/flock_manager.h"
#include "common/human.h"
#include "common/aircraft.h"
#include "common/camera_common.h"

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
    void (model::*on_run)  (net_layer::msg::run_msg const& msg)         = &model::on_inject_msg;
    void (model::*on_malf) (net_layer::msg::malfunction_msg const& msg) = &model::on_inject_msg;

    msg_disp()
        .add<net_layer::msg::run_msg >(boost::bind(on_run      , this, _1))
        .add<net_layer::msg::malfunction_msg >(boost::bind(on_malf      , this, _1))
        ;
}

void model::on_inject_msg(net_layer::msg::run_msg const& msg)
{
     if(msg.ext_id>0 )                          
     {
         auto a = regs_objects_[msg.ext_id];
         
         if(auto pa = aircraft::model_ext_control_ptr(a))
         {
             auto it = last_msg_.find(msg.ext_id);
             if(it!=last_msg_.end() && msg.reverse != it->second.reverse)
             {
                 //pa->set_reverse(msg.reverse);
             }
             
             cg::cpr vc(msg.orien.cpr());
             pa->set_desired  (msg.time,msg.keypoint,msg.reverse?cg::cpr(cg::norm360(msg.orien.get_course() + msg.reverse * 180), msg.orien.get_pitch(), msg.orien.get_roll()):msg.orien,msg.speed, msg.ac);
             
#if 0
             force_log fl;
             LOG_ODS_MSG( "on_inject_msg(net_layer::msg::run_msg const& msg) target_pos.pos : "
                 << "    msg.time: " << msg.time 
                 << "    msg.ac: "   << unsigned(msg.ac)
                 //<< "    x: "       << target_pos.pos.x 
                 //<< "    y: "       << target_pos.pos.y  
                 //<< "    z: "       << target_pos.pos.z 
                 //<< "    course: "  << target_pos.orien.get_course()
                 //<< "    pitch: "   << target_pos.orien.get_pitch()
                 << "\n" );
#endif

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
         else if (flock::manager::model_ext_control_ptr pf = flock::manager::model_ext_control_ptr(a))
         {
             auto it = last_msg_.find(msg.ext_id);

             pf->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pf->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth );         
         }        
         else if (human::model_control_ptr pf = human::model_control_ptr(a))
         {
             auto it = last_msg_.find(msg.ext_id);

             pf->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
             pf->set_ext_wind (msg.mlp.wind_speed, msg.mlp.wind_azimuth );         
         }       
         else if (camera_object::model_ext_control_ptr pf = camera_object::model_ext_control_ptr(a))
         {
             auto it = last_msg_.find(msg.ext_id);

             pf->set_desired  (msg.time,msg.keypoint,msg.orien,msg.speed);
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
		add_object(info);
    }
    else if (vehicle::model_info_ptr info = object)
    {
        add_object(info);
    }
    else if (flock::manager::info_ptr info = object)
    {
        add_object(info);
    }
    else if (human::model_info_ptr info = object)
    {
        add_object(info);
    }
    else if (camera_object::info_ptr info = object)
    {
        add_object(info);
    }
}

void model::on_object_destroying(object_info_ptr object)
{
	auto a = regs_objects_.find(object->object_id());
	if ( a != regs_objects_.end())
	{
		regs_objects_.erase(object->object_id());
	}
}

bool model::add_object(object_info_ptr object)
{
	size_t id = object->object_id();

	regs_objects_[id] = object;

	return true;
}

} // end of objects_reg