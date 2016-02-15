#include "stdafx.h"
#include "precompiled_objects.h"

#include "human_model.h"
//#include "common/collect_collision.h"
#include "geometry/filter.h"
#include "phys/sensor.h"
//  FIXME
// #include "phys/human.h"



namespace human
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc, dict));
}

AUTO_REG_NAME(human_model, model::create);

model::model(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
    , phys_object_model_base(collection_)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    , airports_manager_(find_first_object<airports_manager::info_ptr>(collection_))
    , airport_(airports_manager_->find_closest_airport(pos()))
    , max_speed_(0)
	, start_follow_(false)
{
    FIXME(local global)
    if(root_->position().is_local())    
    {
        root_next_pos_   = geo_position(root_->position().local(),get_base()).pos;
        root_next_orien_ = root_->position().local().orien;
    }
    else
    {
        root_next_pos_   =  root_->position().global().pos;
        root_next_orien_ =  root_->position().global().orien;
    }

    body_node_      = nodes_manager_->find_node("body");

    if (!settings_.route.empty())
        follow_route(settings_.route);

    msg_disp()
        .add<msg::go_to_pos_data            >(boost::bind(&model::on_go_to_pos              , this, _1))
        .add<msg::follow_route_msg_t        >(boost::bind(&model::on_follow_route           , this, _1))
        .add<msg::follow_trajectory_msg_t   >(boost::bind(&model::on_follow_trajectory , this, _1))
        ;

	_targetSpeed = rnd_.random_range(/*settings._minSpeed*/6.f ,/* settings._maxSpeed*/10.f);
    
    geo_point_3 init_pos = geo_point_3(pos(),10);
	set_state(state_t(init_pos, cpr(0), 10.0)); 
    desired_position_  =  pos();  

}

nodes_management::node_info_ptr model::get_root()
{
       return nodes_manager_->find_node("root");
}

void model::update( double time )
{   
    view::update(time);

	if (!phys_model_)
	{
		create_phys();
		sync_phys(0);
	}
    
	if (cg::eq(speed(),0.0))
		idle();
    else if (speed() > 0 && speed() < 1.8 )
        walk();
    else if(speed()>1.8)
        run();

   

    double dt = time - (last_update_ ? *last_update_ : 0);

    if (!cg::eq_zero(dt))
    {
        FIXME(Туфта жесткая) 
        if ( traj_ && start_follow_ && traj_->base_length() - time < -0.1 )
        {
            model::on_follow_trajectory(0);
            start_follow_ = false;
        }

        update_model(dt);

        sync_phys(dt);
 
		sync_nodes_manager(dt);

        last_update_ = time;
    }
}

void model::on_object_created(object_info_ptr object)
{
    view::on_object_created(object);
    if (simple_route::info_ptr is_route = object)
    {
        if (object->name() == settings_.route)
            follow_route(settings_.route);
    }
}

void model::on_object_destroying(object_info_ptr object)
{
    view::on_object_destroying(object);
    if (simple_route::info_ptr is_route = object)
    {
        if (object->name() == settings_.route)
            detach_cur_route();
    }
}

//
//    model_info
//

//phys::rigid_body_ptr model::get_rigid_body() const
//{
//    return phys_aircraft_ ? phys_aircraft_->get_rigid_body() : phys::rigid_body_ptr();
//}

geo_position model::get_phys_pos() const
{
        // FIXME physics

#if 0
    cg::geo_base_3 base = phys_->get_base(*phys_zone_); 
    decart_position cur_pos = phys_model_->get_position();
    geo_position cur_glb_pos(cur_pos, base);

    return cur_glb_pos;
#endif

   return geo_position();
}


//
//    model_control
//

void model::set_desired        (double time, const cg::point_3& pos, const cg::quaternion& orien, const double speed )
{
    decart_position target_pos;

    target_pos.pos   = pos;
    target_pos.orien = orien;
    geo_position gtp(target_pos, get_base());


    if(!traj_)
    {
       traj_ = fms::trajectory::create();
	   start_follow_ = true;
    }

    traj_->append(time, pos, orien, speed);
}

void model::set_ext_wind       (double speed, double azimuth) 
{
    FIXME(Need some wind)
}


void model::on_follow_route(uint32_t route_id)
{
    simple_route::info_ptr routeptr = collection_->get_object(route_id);
    if (routeptr)
        /*state_*/model_state_ = boost::make_shared<follow_route_state>(routeptr);
}

void model::on_follow_trajectory(uint32_t /*route_id*/)
{
    if (traj_)
        /*state_*/model_state_ = boost::make_shared<follow_traj_state>();
}
 

void model::on_go_to_pos(msg::go_to_pos_data const& data)
{
    if (airport_)
    {
//        auto path = airport_->find_shortest_path(pos(), data.pos, aerotow_ ? (1 << ani::TAXI) : (1 << ani::SERVICE));
//        if (!path.empty())
//        {
//            cg::geo_curve_2 curve(path);
//            state_ = boost::make_shared<follow_curve_state>(curve, data.course, !!aerotow_);
//            return;
//        }
    }
        
    model_state_ = boost::make_shared<go_to_pos_state>(data.pos, data.course, /*!!aerotow_*/false);
}

// FIXME это лишнее работаем через см выше
void model::go_to_pos(  cg::geo_point_2 pos, double course )
{
    model_state_ = boost::make_shared<go_to_pos_state>(pos, course, /*!!aerotow_*/false);
}

fms::trajectory_ptr  model::get_trajectory()
{
    return traj_;
} 

void model::follow_route(std::string const& route)
{
    object_info_ptr routeptr = find_object<object_info_ptr>(collection_, route);
    if (routeptr)
    {
        on_follow_route(routeptr->object_id());
    }
}

void model::detach_cur_route()
{
    Assert(follow_route_state_ptr(model_state_) != nullptr);
    model_state_.reset();
}

void model::set_max_speed(double max_speed)
{
    max_speed_ = max_speed;
}

cg::geo_point_2 model::phys_pos() const
{
    // FIXME physics
    //if (phys_model_)
    //{
    //    decart_position cur_pos = phys_model_->get_position();
    //    cg::geo_base_3 base = phys_->get_base(*phys_zone_);
    //    geo_position cur_glb_pos(cur_pos, base);
    //    return cur_glb_pos.pos;
    //}
    return pos();
}

void model::sync_phys(double dt)
{

#if 0
    if (!phys_model_ || !phys_)
        return;

//        double const max_break_accel = aerotow_ ? 2 : 20;
    double const max_accel = 15;
//        double const smooth_factor = 5.;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    decart_position cur_pos = phys_model_->get_position();
    geo_position cur_glb_pos(cur_pos, base);
    double cur_speed = cg::norm(cur_pos.dpos);
    double cur_course = cur_pos.orien.cpr().course;


    cg::point_2 loc_cur_dpos = cg::point_2(cur_pos.dpos) * cg::rotation_2(cur_pos.orien.get_course());
    double cur_speed_signed = loc_cur_dpos.y < 0 ? -cur_speed : cur_speed;

    double const prediction = 25;

    cg::geo_base_2 tgt_pos = pos();
    cg::geo_base_2 predict_tgt_pos = tgt_pos(dpos() * prediction * sys_->calc_step());

    double dist2target = cg::distance((cg::geo_point_2 &)cur_glb_pos.pos, predict_tgt_pos);
    cg::point_2 offset = cg::point_2(cur_glb_pos.pos(predict_tgt_pos));

    cg::point_2 loc_offset = offset * cg::rotation_2(cur_glb_pos.orien.get_course());
    double dist2target_signed = loc_offset.y < 0 ? -dist2target : dist2target;


    double desired_course = cg::polar_point_2(cur_glb_pos.pos(predict_tgt_pos)).course;

    if (dist2target_signed < 0)
        desired_course = cg::norm180(180 + desired_course);

    double steer = cg::bound(cg::norm180(desired_course - cur_course),-30., 30.);
    if (dist2target_signed < 0.5)
        steer = 0;
    double max_speed = cg::clamp(0., 30., max_speed_, 1.5)(fabs(steer));


    double desired_speed_signed = filter::BreakApproachSpeed(0., dist2target_signed, cur_speed_signed, max_speed, max_accel, sys_->calc_step(), prediction);
//         if (cg::ge(cg::sqr(cur_speed), 2 * max_break_accel * dist2target)) // need braking
//             desired_speed = filter::BreakApproachSpeed(0., dist2target, cur_speed, max_speed, max_break_accel, sys_->calc_step(), smooth_factor);
//         else
//             desired_speed = filter::ApproachSpeed(0., cg::norm(offset), cur_speed, max_speed, max_accel, sys_->calc_step(), smooth_factor);

    if (fabs(desired_speed_signed) < 0.1)
        desired_speed_signed = 0;

    double brake = 0;

    double max_thrust = aerotow_ ? 100 : 10;
    double thrust = cg::bound(0.1 * (desired_speed_signed - cur_speed_signed) / (5 * sys_->calc_step()), -max_thrust, max_thrust);

    if (fabs(cur_speed_signed) > fabs(desired_speed_signed))
    {
        brake = cg::clamp(0., 10., 0., 1.)(fabs(cur_speed_signed - desired_speed_signed));
        thrust = 0;
    }


    phys_model_->set_steer(steer);
    phys_model_->set_thrust(thrust);
    phys_model_->set_brake(brake);

    if (aerotow_)
    {
        if ( aircraft::model_info_ptr(aerotow_)->tow_attached())
        {                          
            aircraft::model_control_ptr(aerotow_)->set_brake(brake*100);
        }
    }  

    logger::need_to_log(false);

    LOG_ODS_MSG ("sync_phys  steer:  "  << steer 
                 << "    thrust: " << thrust 
                 << "    brake: " << brake 
                 << "\n");

    logger::need_to_log(false);
#else
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
    cg::geo_point_3 d_p = base(base(cur_glb_pos.pos) + cur_pos.dpos);

	cg::polar_point_3 cp (cg::geo_base_3(/*desired_position_*/d_p)(cur_glb_pos.pos));
	cpr cpr_des =  cg::cpr(cp.course,0/*cp.pitch*/,0);
	quaternion  desired_orien_(cpr_des);  

	point_3 forward_dir = -cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 1, 0))) ;
	point_3 right_dir   =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(1, 0, 0))) ;
	point_3 up_dir      =  cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 0, 1))) ;

    point_3 omega_rel     =   cg::get_rotate_quaternion(cur_glb_pos.orien, desired_orien_).rot_axis().omega() * /*_damping*/1.f * (dt);

#if 1
	if(_targetSpeed > -1){
		phys::character::control_ptr(phys_model_)->set_angular_velocity(omega_rel);
	}

    FIXME(Перенести ограничение в более подходящее место)
	phys::character::control_ptr(phys_model_)->set_linear_velocity(/*forward_dir*/ point_3(1.0, 1.0, /*0.001*/0.0)* cg::bound(speed(), 0.0,4.305556)  );
#endif

#endif
    //if (settings_.debug_draw)
    //    send_cmd(msg::phys_pos_msg(cur_glb_pos.pos, cur_course));
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

void model::settings_changed()
{
    view::settings_changed();
}

void model::update_model( double dt )
{
    cg::geo_base_2 cur_pos = pos();
     // FIXME physics
#if 0
    if (phys_model_ && phys_) // callback to phys pos
    {
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
        decart_position phys_pos = phys_model_->get_position();
        geo_position glb_phys_pos(phys_pos, base);

        double dist = cg::distance((cg::geo_point_2 &)glb_phys_pos.pos, cur_pos);
        // FIXME state устанавливется через чарт
        // у меня координаты отличаются надо думать
        if (dist > 10)
            return;
    }
#endif
    if (model_state_)
    {
        model_state_->update(this, dt);
        if (model_state_->end())
        {
            model_state_.reset();
            set_state(state_t(pos(), orient(), 0.0));
        }
    }
}

void model::on_zone_created( size_t /*id*/ )
{
    //create_phys_human();        
}

void model::on_zone_destroyed( size_t id )
{  
    // FIXME physics
#if 0
    if (phys_zone_ && *phys_zone_ == id)
    {
        phys_model_.reset();
    }
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
	phys::compound_sensor_ptr s = phys::character::fill_cs(nodes_manager_); 
	decart_position p(base(state_.pos), state_.orien);

	phys::character::params_t  params;
	params.mass = 1;
	phys_model_ = phys_->get_system(*phys_zone_)->create_character(params, s, p);

}

void model::go(cg::polar_point_2 const &offset)
{
    model_state_ = boost::make_shared<go_to_pos_state>(cg::geo_offset(pos(), offset), boost::none, /*!!aerotow_*/false);
}


void model::idle()
{
	// auto const& settings = _spawner->settings();
	if(anim_state_ != as_idle)
	{ 
		root_->play_animation("idle", 1.0 / rnd_.random_range(/*settings._minAnimationSpeed*/0.1f, /*settings._maxAnimationSpeed*/0.4f), -1., -1., 0.0);
		anim_state_ = as_idle;
	}

	// desired_position_ = geo_base_3(_spawner->pos())(rnd_.inside_unit_sphere () * settings._spawnSphere) ;

}

void model::walk()
{
	// auto const& settings = _spawner->settings();
	if(anim_state_ != as_walk)
	{ 
		root_->play_animation("walk", 1.0 / rnd_.random_range(/*settings._minAnimationSpeed*/0.5f, /*settings._maxAnimationSpeed*/1.f), -1., -1., 0.0);
		anim_state_ = as_walk;
	}

	// desired_position_ = geo_base_3(_spawner->pos())(rnd_.inside_unit_sphere () * settings._spawnSphere) ;

}

void model::run()
{
	// auto const& settings = _spawner->settings();
	if(anim_state_ != as_run)
	{ 
		root_->play_animation("run", 1.0 / rnd_.random_range(/*settings._minAnimationSpeed*/0.8f, /*settings._maxAnimationSpeed*/1.5f), -1., -1., 0.0);
		anim_state_ = as_run;
	}

	// desired_position_ = geo_base_3(_spawner->pos())(rnd_.inside_unit_sphere () * settings._spawnSphere) ;

}

} // human 


