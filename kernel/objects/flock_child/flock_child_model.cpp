#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_child_model.h"
//#include "common/collect_collision.h"
#include "phys/sensor.h"

namespace flock
{

namespace child
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

AUTO_REG_NAME(flock_child_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict )
    : view                  (oc, dict)
    , phys_object_model_base(collection_)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    , _speed                (10.0 )
	, _soar                 (true )
    , _landing              (false)
	, _flatFlyDown          (true )
    , _stuckCounter         (0.0  )
    , _landingSpotted       (false)
{
    settings_._avoidValue = rnd_.random_range(.2, .4/*.3, .1*/);
    create_phys();

	start_ = boost::bind(&model::wander, this, 0.0f );
}

void model::update(double time)
{
    view::update(time);
	
	if(start_)
	{
		start_();
		start_.clear();
	}

	double dt = time - (last_update_ ? *last_update_ : 0);
    if (!cg::eq_zero(dt))
    {

        update_model(time, dt);
#if 0
        if (!manual_controls_)
#endif
        sync_phys(dt);
#if 0
        else
        {
            cg::geo_base_3 base = phys_->get_base(*phys_zone_); 
            decart_position cur_pos = phys_vehicle_->get_position();
            geo_position cur_glb_pos(cur_pos, base);
            set_state(state_t(cur_glb_pos.pos, cur_pos.orien.get_course(), 0)); 
        }
#endif


        sync_nodes_manager(dt);

    }

    last_update_ = time;

}

void model::on_child_removing(object_info_ptr child)
{
    view::on_child_removing(child);

#if 0
    if (nodes_manager_ == child)
        nodes_manager_.reset();
#endif
}


void model::create_phys()
{
    if (!phys_ || !root_)
        return;

    phys_zone_ = phys_->get_zone(cg::geo_point_3(pos(), 0));
    if (!phys_zone_)
        return;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    //phys::sensor_ptr s = collect_collision(nodes_manager_, body_node_);
    phys::compound_sensor_ptr s = phys::flock::fill_cs(nodes_manager_); 
    decart_position p(base(state_.pos), state_.orien);
    
    phys::flock::params_t  params;
    params.mass = 1;
    phys_model_ = phys_->get_system(*phys_zone_)->create_flock_child(params, s, p);

}

void model::update_model( double time, double dt )
{
    cg::geo_base_3 cur_pos = pos();

    if (phys_ && phys_model_) // callback to phys pos
    {
        manager::info_ptr spawner = _spawner.lock();
        if(spawner)
        {
            const cg::geo_base_3& base = phys_->get_base(*phys_zone_);
            decart_position phys_pos = phys_model_->get_position();
            geo_position glb_phys_pos(phys_pos, base);
            auto const& settings = spawner->settings();

            double dist = cg::distance(glb_phys_pos.pos, desired_position_/*cur_pos*/);

            //Soar Timeout - Limits how long a bird can soar
            if(_soar && settings._soarMaxTime > 0){ 		
                if(_soarTimer >settings._soarMaxTime){
                    flap();
                    _soarTimer = 0;
                }else {
                    _soarTimer+=dt;
                }
            }

            if(!_landingSpotted && dist < settings._waypointDistance +_stuckCounter){
                wander(0);	//create a new waypoint
                _stuckCounter=0;
            }else{
                _stuckCounter += dt;
            }
        }
    }


}

void model::sync_phys(double dt)
{
    if (!phys_model_ || !phys_)
        return;

    point_3     wind(0.0,0.0,0.0);

    double const max_accel = 15;

	cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    decart_position cur_pos = phys_model_->get_position();

    geo_position cur_glb_pos(cur_pos, base);
    double cur_speed  = cg::norm(cur_pos.dpos);
    double cur_course = cur_pos.orien.cpr().course;
    double cur_roll   = cur_pos.orien.cpr().roll;
	
	// ?? cg::geo_direction
    
    cg::polar_point_3 cp (cg::geo_base_3(desired_position_)(cur_glb_pos.pos));
    cpr cpr_des =  cg::cpr(cp.course,cp.pitch);
    quaternion  desired_orien_(cpr_des);  

    point_3 forward_dir = -cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 1, 0))) ;
    point_3 right_dir   =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(1, 0, 0))) ;
    point_3 up_dir      =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 0, 1))) ;

#if 0
    point_3 vk = cur_pos.dpos - wind;

    point_3 Y = !cg::eq_zero(cg::norm(vk)) ? cg::normalized(vk) : forward_dir;
    point_3 Z = cg::normalized_safe(right_dir ^ Y);
    point_3 X = cg::normalized_safe(Y ^ Z);
    cg::rotation_3 vel_rotation(X, Y, Z);
    
    cg::rotation_3 rot(desired_orien_.cpr());

    cg::geo_base_3 predict_pos = /*cur_glb_pos.pos*/desired_position_;//; FIXME desired_position_;
    cg::geo_base_3 predict_tgt_pos = predict_pos(rot * cg::transform_4().inverted().translation()/* * body_transform_inv_.inverted().translation()*/);  // FIXME   *body_transform_inv_.inverted().translation()
    
    double dist2target = cg::distance2d(cur_glb_pos.pos, predict_tgt_pos);
    point_2 offset = cg::point_2(cur_glb_pos.pos(predict_tgt_pos));

    point_3 Y_right_dir_proj =  Y - Y * right_dir * right_dir;
    double attack_angle = cg::rad2grad(cg::angle(Y_right_dir_proj, forward_dir)) * (-cg::sign(Y * up_dir));
    double slide_angle = cg::rad2grad(cg::angle(Y, Y_right_dir_proj))  * (-cg::sign(Y * right_dir));
#endif

    point_3 omega_rel     =   cg::get_rotate_quaternion(cur_glb_pos.orien, desired_orien_).rot_axis().omega() * _damping * (dt);


    
	if(_targetSpeed > -1){
		phys::flock::control_ptr(phys_model_)->set_angular_velocity(omega_rel);
	}

    phys::flock::control_ptr(phys_model_)->set_linear_velocity(forward_dir * _speed );

}

void model::sync_nodes_manager( double /*dt*/ )
{
    if (phys_model_ && root_)
    {
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
        decart_position bodypos = phys_model_->get_position();
        decart_position root_pos = bodypos /** body_transform_inv_*/;// FIXME Модельно зависимое решение 

        geo_position pos(root_pos, base);

        // FIXME Глобальные локальные преобразования 
        nodes_management::node_position root_node_pos = root_->position();
        root_node_pos.global().pos = root_next_pos_;
        root_node_pos.global().dpos = cg::geo_base_3(root_next_pos_)(pos.pos) / (sys_->calc_step());

        root_node_pos.global().orien = root_next_orien_;
        root_node_pos.global().omega = cg::get_rotate_quaternion(root_node_pos.global().orien, pos.orien).rot_axis().omega() / (sys_->calc_step());

        // nodes_management::node_position rnp = local_position(0,0,cg::geo_base_3(get_base())(root_node_pos.global().pos),root_node_pos.global().orien);
        root_->set_position(root_node_pos);

        root_next_pos_ = pos.pos;
        root_next_orien_ = pos.orien;

    }
}


void model::flap()
{
    manager::info_ptr spawner = _spawner.lock();
    if(spawner)
    {
        auto const& settings = spawner->settings();
        if(flock_state_ != fl_flap)
        { 

            root_->play_animation("flap", 1.0 / rnd_.random_range(settings._minAnimationSpeed, settings._maxAnimationSpeed), -1., -1., 0.0);
            _dived = false;
            flock_state_ = fl_flap;
        }

        desired_position_ = geo_base_3(spawner->pos())(rnd_.inside_unit_sphere () * settings._spawnSphere) ;
    }
}

void model::soar()
{
    manager::info_ptr spawner = _spawner.lock();
    if(spawner)
    {
        auto const& settings = spawner->settings();
	    if(flock_state_ != fl_soar)
	    { 
		    root_->play_animation("soar", 1.0 / rnd_.random_range(settings._minAnimationSpeed, settings._maxAnimationSpeed), -1., -1., /*1.5*/0.0);
		    _soar = true;
	
		    flock_state_ = fl_soar;
	    }

        desired_position_ = geo_base_3(spawner->pos())(rnd_.inside_unit_sphere () * settings._spawnSphere) ;
    }
}

void model::dive()
{
    manager::info_ptr spawner = _spawner.lock();
    if(spawner)
    {
        auto const& settings = spawner->settings();
	    if(flock_state_ != fl_dive)
	    { 	
		    root_->play_animation("soar", 1.0 / rnd_.random_range(settings._minAnimationSpeed, settings._maxAnimationSpeed), -1., -1., /*1.5*/0.0);

		    _dived = true;
		     flock_state_ = fl_dive;
	    }

	    cg::point_3 pos_us = rnd_.inside_unit_sphere () * settings._spawnSphere; 
	    pos_us.z = rnd_.random_range(-settings._spawnSphere *.5 , settings._spawnSphere *.5) - settings._diveValue; 
        desired_position_ = geo_base_3(spawner->pos())(pos_us) ;
    }
}

void model::wander(float delay)
{
    manager::info_ptr spawner = _spawner.lock();
    if(spawner)
    {
        auto const& settings = spawner->settings();

	    // yield(WaitForSeconds(delay));
	    _damping = rnd_.random_range(settings._minDamping, settings._maxDamping);
	    _targetSpeed = rnd_.random_range(settings._minSpeed, settings._maxSpeed);
	    _lerpCounter = 0;
	
	    double r = rnd_.random_range(0.0, 1.0);

	    if(/*!settings._soarAnimation.empty() &&*/!_flatFlyDown && !_dived && r < settings._soarFrequency){
		    soar();
	    }else if(!_flatFlyDown && !_dived && rnd_.random_range(0.0, 1.0) < settings._diveFrequency){	
		    dive();
	    }else{
		    if(!_landing){
			    flap();
		    }
	    }
    }
}

}

} // end of flock
