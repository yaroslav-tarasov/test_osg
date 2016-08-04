#include "stdafx.h"
#include "precompiled_objects.h"

#include "arresting_gear_model.h"
//#include "common/collect_collision.h"
#include "phys/sensor.h"

namespace arresting_gear
{


object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

AUTO_REG_NAME(arresting_gear_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict )
    : view                  (oc, dict)
    , phys_object_model_base(collection_)
    , sys_(dynamic_cast<model_system *>(oc.sys))
{
    
}

void model::update(double time)
{
    view::update(time);
	
    create_phys();

	double dt = time - (last_update_ ? *last_update_ : 0);
    if (!cg::eq_zero(dt))
    {

        update_model(time, dt);

        sync_phys(dt);

        sync_nodes_manager(dt);

    }

    last_update_ = time;

}

void model::on_child_removing(object_info_ptr child)
{
    view::on_child_removing(child);

    if (nodes_manager_ == child)
        nodes_manager_.reset();
}


void model::create_phys()
{
    if ( !phys_ || phys_model_ /*|| !root_*/)
        return;

    phys_zone_ = phys_->get_zone(cg::geo_point_3(pos(), 0));
    if (!phys_zone_)
        return;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

#if 0
    phys::compound_sensor_ptr s = phys::arresting_gear::fill_cs(nodes_manager_); 
#endif
    decart_position p(base(state_.pos), state_.orien);

    phys::arresting_gear::params_t  params;
    
    params.seg_num   = 25;

#if 1
    params.ropes.push_back(std::make_pair( cg::point_3f( -2342.90, -368.16 ,0.17), cg::point_3f( -2346.57, -338.91 ,0.17) )); 
    params.ropes.push_back(std::make_pair( cg::point_3f( -2330.30, -366.48 ,0.17), cg::point_3f( -2333.98, -337.22 ,0.17) ));
#endif
#if 1
    params.ropes.push_back(std::make_pair( cg::point_3f( -2318.46, -365.02 ,0.17), cg::point_3f( -2322.13, -335.76 ,0.17) ));
    params.ropes.push_back(std::make_pair( cg::point_3f( -2305.79, -363.45 ,0.17), cg::point_3f( -2309.47, -334.20 ,0.17) ));   // 12.7669
#endif


    phys_model_ = phys_->get_system(*phys_zone_)->create_arresting_gear(params, phys::compound_sensor_ptr(), p);

}

void model::update_model( double time, double dt )
{
    cg::geo_base_3 cur_pos = pos();

    if (phys_ && phys_model_) // callback to phys pos
    {
        const cg::geo_base_3& base = phys_->get_base(*phys_zone_);
#if 0
        decart_position phys_pos = phys_model_->get_position();
        geo_position glb_phys_pos(phys_pos, base);

        double dist = cg::distance(glb_phys_pos.pos, desired_position_/*cur_pos*/);
#endif

        update_ropes(time);
    }

}

void model::update_ropes(double time)
{
    if (!phys_model_)
        return;

    auto ropes_info = std::move(phys_model_->get_ropes_info());
    if (!ropes_info.empty())
    {
        vector<rope_state_t> ropes_state_to_send;
        ropes_state_to_send.reserve(ropes_info.size());
        for (auto it = ropes_info.begin(); it != ropes_info.end(); ++it)
        {
             FIXME(Some kind of on may be placed here)
             if (time  < 40.)
                (*it).on = false;
             else
                (*it).on = true;

             ropes_state_to_send.push_back(std::move(*it));
        }

        set(msg::ropes_state(move(ropes_state_to_send), time), false);
    }

 
}

void model::sync_phys(double dt)
{
    if ( !phys_ || !phys_model_)
        return;

    point_3     wind(0.0,0.0,0.0);

    double const max_accel = 15;

	cg::geo_base_3 base = phys_->get_base(*phys_zone_);

#if 0
    decart_position cur_pos = phys_model_->get_position();

    geo_position cur_glb_pos(cur_pos, base);
    double cur_speed  = cg::norm(cur_pos.dpos);
    double cur_course = cur_pos.orien.cpr().course;
    double cur_roll   = cur_pos.orien.cpr().roll;
	
	// ?? cg::geo_direction
    
    cg::polar_point_3 cp (cg::geo_base_3(desired_position_)(cur_glb_pos.pos));
    cpr cpr_des =  cg::cpr(cp.course,cp.pitch);
    quaternion  desired_orien_(cpr_des);  

    point_3 forward_dir =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 1, 0))) ;
    point_3 right_dir   =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(1, 0, 0))) ;
    point_3 up_dir      =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 0, 1))) ;

    point_3 omega_rel     =   cg::get_rotate_quaternion(cur_glb_pos.orien, desired_orien_).rot_axis().omega() * _damping * (dt);


    
	if(_targetSpeed > -1){
		phys::arresting_gear::control_ptr(phys_model_)->set_angular_velocity(omega_rel);
	}

    phys::arresting_gear::control_ptr(phys_model_)->set_linear_velocity(forward_dir * _speed );
#endif

}

void model::sync_nodes_manager( double /*dt*/ )
{
    if (phys_model_ && root_)
    {
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
#if 0
        decart_position bodypos = phys_model_->get_position();
        decart_position root_pos = bodypos /** body_transform_inv_*/;// FIXME Модельно зависимое решение 

        geo_position pos(root_pos, base);

        // FIXME Глобальные локальные преобразования 
        nodes_management::node_position root_node_pos = root_->position();
        //root_node_pos.global().pos = root_next_pos_;
        root_node_pos.global().dpos = cg::geo_base_3(root_node_pos.global().pos)(pos.pos) / (sys_->calc_step());

        //root_node_pos.global().orien = root_next_orien_;
        root_node_pos.global().omega = cg::get_rotate_quaternion(root_node_pos.global().orien, pos.orien).rot_axis().omega() / (sys_->calc_step());

        // nodes_management::node_position rnp = local_position(0,0,cg::geo_base_3(get_base())(root_node_pos.global().pos),root_node_pos.global().orien);
        root_->set_position(root_node_pos);
#endif

    }
}

void model::on_target_changed (const boost::optional<uint32_t> &  old_id, const boost::optional<uint32_t> & id )
{
    if (!phys_model_)
        return;

    if (target_id_ )
    {
		auto target_ptr = aircraft::model_info_ptr(collection_->get_object(*target_id_));
			
		if(target_ptr->get_rigid_body())	
		{
			cg::point_3 offset;

			FIXME(tow_offset)
			phys::arresting_gear::control_ptr(phys_model_)->set_target(target_ptr->get_rigid_body(), target_ptr->tow_offset(), offset);
		}
    }
    else
    {
        // phys::arresting_gear::control_ptr(phys_model_)->reset_target();


    }

}

} // end of arresting_gear
