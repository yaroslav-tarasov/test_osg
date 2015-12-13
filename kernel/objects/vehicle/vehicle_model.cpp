#include "stdafx.h"
#include "precompiled_objects.h"

#include "vehicle_model.h"
//#include "common/collect_collision.h"
#include "geometry/filter.h"
#include "phys/sensor.h"
#include "phys/vehicle.h"



namespace vehicle
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc, dict));
}

AUTO_REG_NAME(vehicle_model, model::create);

model::model(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
    , phys_object_model_base(collection_)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    //, ani_(find_first_object<ani_object::info_ptr>(collection_))
    //, airport_(ani_->navigation_info()->find_airport(pos()))
    , airports_manager_(find_first_object<airports_manager::info_ptr>(collection_))
    , airport_(airports_manager_->find_closest_airport(pos()))
    , manual_controls_(false)
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
    FIXME(Надо решить нужна ли здесь инициализация см view, как впрочем и дублирование переменных) 
    //tow_point_node_ = nodes_manager_->find_node("tow_point");
    //rtow_point_node_ = nodes_manager_->find_node("rtow_point");

    if (!settings_.route.empty())
        follow_route(settings_.route);

    msg_disp()
        .add<msg::attach_tow_msg_t          >(boost::bind(&model::on_attach_tow             , this, _1))
        .add<msg::detach_tow_msg_t          >(boost::bind(&model::on_detach_tow             , this))
        .add<msg::go_to_pos_data            >(boost::bind(&model::on_go_to_pos              , this, _1))
        .add<msg::follow_route_msg_t        >(boost::bind(&model::on_follow_route           , this, _1))
        .add<msg::brake_msg_t               >(boost::bind(&model::on_brake                  , this, _1))
        .add<msg::reverse_msg_t             >(boost::bind(&model::on_reverse                , this, _1))

        .add<msg::follow_trajectory_msg_t        >(boost::bind(&model::on_follow_trajectory , this, _1))
//        .add<msg::debug_controls_data       >(boost::bind(&model::on_debug_controls         , this, _1))
//        .add<msg::disable_debug_ctrl_msg_t  >(boost::bind(&model::on_disable_debug_controls , this, _1))
        ;

    //create_phys_vehicle();
}

nodes_management::node_info_ptr model::get_root()
{
       return nodes_manager_->find_node("root");
}

void model::update( double time )
{   
    view::update(time);

    if (!phys_vehicle_)
    {
        create_phys_vehicle();
        sync_phys();
    }

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

		if (!manual_controls_)
			sync_phys();
		else
		{
			cg::geo_base_3 base = phys_->get_base(*phys_zone_); 
			decart_position cur_pos = phys_vehicle_->get_position();
			geo_position cur_glb_pos(cur_pos, base);
			set_state(state_t(cur_glb_pos.pos, cur_pos.orien.get_course(), 0)); 
		}

        sync_nodes_manager(dt);

        if (aerotow_)
        {
            if (aircraft::model_info_ptr(aerotow_)->get_rigid_body() && phys_vehicle_ && aircraft::model_info_ptr(aerotow_)->tow_attached())
            {
                rod_course   = cg::norm180(phys_vehicle_->get_tow_rod_course());
                air_course   = cg::norm180(aircraft::model_info_ptr(aerotow_)->get_phys_pos().orien.cpr().course);
                steer_course = cg::norm180(rod_course - air_course);

                aircraft::model_control_ptr(aerotow_)->set_steer(steer_course);
            }
            else
            {
                on_detach_tow();
            }
        }


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

FIXME("Для выталкивания добавлен впустую ряд функций, необходимо развить")
//phys::rigid_body_ptr model::get_rigid_body() const
//{
//    return phys_aircraft_ ? phys_aircraft_->get_rigid_body() : phys::rigid_body_ptr();
//}

point_3 model::tow_offset() const
{
    return tow_point_node_ ? nodes_manager_->get_relative_transform( tow_point_node_, body_node_).translation() : point_3();
}

point_3 model::rtow_offset() const
{
    return rtow_point_node_ ? nodes_manager_->get_relative_transform( rtow_point_node_, body_node_).translation() : point_3();
}

bool model::tow_attached() const
{
    return tow_attached_;
}

geo_position model::get_phys_pos() const
{
    cg::geo_base_3 base = phys_->get_base(*phys_zone_); 
    decart_position cur_pos = phys_vehicle_->get_position();
    geo_position cur_glb_pos(cur_pos, base);

    return cur_glb_pos;
}


//
//    model_control
//

void model::set_tow_attached(optional<uint32_t> attached, boost::function<void()> tow_invalid_callback)
{
    if (tow_attached_ == attached)
        return ;

    //tow_attached_ = attached;
    //tow_invalid_callback_ = tow_invalid_callback;
    //if (phys_aircraft_)
    //{
    //    phys_aircraft_->attach_tow(attached);
    //    traj_.reset();
    //}

    //if (!tow_attached_)
    //    sync_fms(true);
}

void model::set_steer( double steer )
{   
    Assert(tow_attached_);

    if (phys_vehicle_)
        phys_vehicle_->set_steer(steer);
}

void model::set_brake( double brake )
{   
    Assert(tow_attached_);

    if (phys_vehicle_)
        phys_vehicle_->set_brake(brake);
}

void model::set_reverse( bool val )
{   
    FIXME("towing only?")
    // Assert(tow_attached_);

    if (phys_vehicle_)
        phys_vehicle_->set_reverse(val);
}

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

void model::on_aerotow_changed( aircraft::info_ptr old_aerotow, bool reverse )
{   
    if (!phys_vehicle_)
        return;

    if (aerotow_ && aircraft::model_info_ptr(aerotow_)->get_rigid_body())
    {
        cg::point_3 tow_offset = reverse
			? (rtow_point_node_ ? nodes_manager_->get_relative_transform(rtow_point_node_, body_node_).translation() : cg::point_3())
			: (tow_point_node_ ? nodes_manager_->get_relative_transform(tow_point_node_, body_node_).translation() : cg::point_3())
			;
		
        phys::ray_cast_vehicle::control_ptr(phys_vehicle_)->set_tow(aircraft::model_info_ptr(aerotow_)->get_rigid_body(), tow_offset, aircraft::model_info_ptr(aerotow_)->tow_offset());

        aircraft::model_control_ptr(aerotow_)->set_tow_attached(object_id(), boost::bind(&model::on_detach_tow, this));
    }
    else
    {
        if (old_aerotow)
            aircraft::model_control_ptr(old_aerotow)->set_tow_attached(boost::none, boost::function<void()>());

        phys::ray_cast_vehicle::control_ptr(phys_vehicle_)->reset_tow();
    }
}

void model::on_attach_tow(/* uint32_t tow_id*/ msg::attach_tow_msg_t const& data)
{
    if (!aerotow_)
        set_tow(data.tow_id,data.reverse);
}

void model::on_detach_tow()
{
    if (aerotow_)
        set_tow(boost::none,false);
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
 
void model::on_brake(double val)
{
    phys_vehicle_->set_brake(val);
}

void model::on_reverse(bool val)
{
     phys_vehicle_->set_reverse(val);
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
        
    model_state_ = boost::make_shared<go_to_pos_state>(data.pos, data.course, !!aerotow_);
}

// FIXME это лишнее работаем через см выше
void model::go_to_pos(  cg::geo_point_2 pos, double course )
{
    model_state_ = boost::make_shared<go_to_pos_state>(pos, course, !!aerotow_);
}

//void model::on_debug_controls(msg::debug_controls_data const& d)
//{
//    if (phys_vehicle_)
//    {
//        state_.reset();
//        phys_vehicle_->set_steer(30 * d.steer);
//        phys_vehicle_->set_thrust(0.1 * d.thrust);
//        phys_vehicle_->set_brake(d.brake);
//        manual_controls_ = true;
//    }
//}

//void model::on_disable_debug_controls(msg::disable_debug_ctrl_msg_t const& /*d*/)
//{
//    if (phys_vehicle_)
//    {
//        phys_vehicle_->set_steer(0.);
//        phys_vehicle_->set_thrust(0.);
//        phys_vehicle_->set_brake(1.);
//        manual_controls_ = false;
//
//        geo_base_3 base = phys_->get_base(*phys_zone_);
//        decart_position cur_pos = phys_vehicle_->get_position();
//        geo_position cur_glb_pos(cur_pos, base);
//
//        set_state(state_t(cur_glb_pos.pos(cur_pos.dpos * 10*0.1), cur_pos.orien.get_course(), 0));
//    }
//}

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

void model::set_course_hard(double course)
{                     
    if (phys_vehicle_)
    {
        phys::ray_cast_vehicle::control_ptr(phys_vehicle_)->set_course_hard(course);
    }
}

cg::geo_point_2 model::phys_pos() const
{
    if (phys_vehicle_)
    {
        decart_position cur_pos = phys_vehicle_->get_position();
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
        geo_position cur_glb_pos(cur_pos, base);
        return cur_glb_pos.pos;
    }
    return pos();
}

void model::sync_phys()
{
    if (!phys_vehicle_ || !phys_)
        return;

//        double const max_break_accel = aerotow_ ? 2 : 20;
    double const max_accel = 15;
//        double const smooth_factor = 5.;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    decart_position cur_pos = phys_vehicle_->get_position();
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


    phys_vehicle_->set_steer(steer);
    phys_vehicle_->set_thrust(thrust);
    phys_vehicle_->set_brake(brake);

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
    //if (settings_.debug_draw)
    //    send_cmd(msg::phys_pos_msg(cur_glb_pos.pos, cur_course));
}


void model::sync_nodes_manager( double /*dt*/ )
{
    
    if (phys_vehicle_ && root_)
    {
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
        decart_position bodypos = phys_vehicle_->get_position();
        decart_position root_pos = bodypos * body_transform_inv_;

#if !defined(OSG_NODE_IMPL) 
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
#endif

        geo_position body_pos(phys_vehicle_->get_position() * body_transform_inv_, base);

        for (size_t i = 0; i < wheels_.size(); ++i)
        {
            geo_position wpos(phys_vehicle_->get_wheel_position(i), base);

            cg::quaternion wpos_rel_orien = (!body_pos.orien) * wpos.orien;
            cg::point_3 wpos_rel_pos = (!body_pos.orien).rotate_vector(body_pos.pos(wpos.pos));

#ifdef OSG_NODE_IMPL
            nodes_management::node_info_ptr rel_node = wheels_[i].node;
#else
            nodes_management::node_info_ptr rel_node = wheels_[i].node->rel_node();
#endif  

            //cg::geo_base_3 global_pos = wheels_[i].node->get_global_pos();
            //cg::quaternion global_orien = wheels_[i].node->get_global_orien();

            cg::transform_4 rel_node_root_tr = rel_node->get_root_transform();

            cg::point_3    desired_pos_in_rel = rel_node_root_tr.inverted() * wpos_rel_pos;
            cg::quaternion desired_orien_in_rel = (!cg::quaternion(rel_node_root_tr.rotation().cpr())) * wpos_rel_orien;

            nodes_management::node_position node_pos = wheels_[i].node->position();

            node_pos.local().dpos.z = (desired_pos_in_rel.z - node_pos.local().pos.z) / (2 *sys_->calc_step());

            cg::point_3 omega_rel     = cg::get_rotate_quaternion(node_pos.local().orien, desired_orien_in_rel).rot_axis().omega() / (sys_->calc_step());

            node_pos.local().omega = omega_rel;

#ifdef OSG_NODE_IMPL
            // FIXME отсутствие промежуточной логики приводит к странным решениям
            node_pos.local().orien = desired_orien_in_rel;
#endif

            wheels_[i].node->set_position(node_pos);
        }


    }
}

void model::settings_changed()
{
    view::settings_changed();

    if (!aerotow_ || (object_info_ptr(aerotow_)->name() != settings_.aerotow))
    {            
        aircraft::model_info_ptr new_aerotow = find_object<aircraft::model_info_ptr>(collection_, settings_.aerotow);

        if (new_aerotow)
            set_tow(kernel::object_info_ptr(new_aerotow)->object_id(), false) ;
    }
}

void model::update_model( double dt )
{
    cg::geo_base_2 cur_pos = pos();

    if (phys_vehicle_ && phys_) // callback to phys pos
    {
        cg::geo_base_3 base = phys_->get_base(*phys_zone_);
        decart_position phys_pos = phys_vehicle_->get_position();
        geo_position glb_phys_pos(phys_pos, base);

        double dist = cg::distance((cg::geo_point_2 &)glb_phys_pos.pos, cur_pos);
        FIXME(state устанавливется через чарт); 
	    // у меня координаты отличаются надо думать
        if (dist > 10)
            return;
    }

    if (model_state_)
    {
        model_state_->update(this, dt);
        if (model_state_->end())
        {
            model_state_.reset();
            set_state(state_t(pos(), course(), 0));
        }
    }
}

void model::on_zone_created( size_t /*id*/ )
{
    //create_phys_vehicle();        
}

void model::on_zone_destroyed( size_t id )
{          
    if (phys_zone_ && *phys_zone_ == id)
    {
        phys_vehicle_.reset();
    }
}

void model::create_phys_vehicle()
{
    using namespace nodes_management;

    if (!phys_ || !root_)
        return;

    phys_zone_ = phys_->get_zone(cg::geo_point_3(pos(), 0));
    if (!phys_zone_)
        return;

    cg::geo_base_3 base = phys_->get_base(*phys_zone_);

    //phys::sensor_ptr s = collect_collision(nodes_manager_, body_node_);
    phys::compound_sensor_ptr s = phys::ray_cast_vehicle::fill_cs(nodes_manager_);

    body_transform_inv_ =  cg::transform_4(); 
    // FIXME TYV  сдается мне нефига не нужный код 
    // В модели симекса съедается трансформ на геометрии, и Body оказывается востребованным
    // get_relative_transform(nodes_manager_, body_node_).inverted();

    phys::collision_ptr collision(phys_->get_system(*phys_zone_));

    // TODO OR NOT
    //auto isection = collision->intersect_first(base(cg::geo_point_3(pos(), 10.)), base(cg::geo_point_3(pos(), -10.)));

    double height = 0;
    //if (isection)   // TODO OR NOT
    //    height += 10 - 20. * *isection;

    cg::transform_4 veh_transform = cg::transform_4(as_translation(base(cg::geo_point_3(pos(), height))), cg::rotation_3(cg::cpr(course(), 0,0))) * body_transform_inv_.inverted();

    //phys::box_sensor_t box(cg::rectangle_3(point_3(-2, -2, -1), point_3(2, 2, 1)));
    decart_position p(veh_transform.translation(), cg::quaternion(veh_transform.rotation().cpr()));
    //p.pos.z -= 0.2;
    p.pos.z = 0;
    FIXME(Масса тел: 2000 или 200 вот вопрос )
    phys_vehicle_ = phys_->get_system(*phys_zone_)->create_ray_cast_vehicle(cfg().model_params.vehicle_mass, s, p);

    // implementation
#ifdef OSG_NODE_IMPL 
    nm::visit_sub_tree(nodes_manager_->get_node(0), [this](nm::node_info_ptr wheel_node)->bool
    {
        std::string name = wheel_node->name();
        if (boost::starts_with(wheel_node->name(), "wheel"))
        {
            // Поиск имени симекса нам не походит
            // cg::transform_4 wt = nm::get_relative_transform(nodes_manager_, wheel_node, this->body_node_);
            cg::transform_4 wt = this->nodes_manager_->get_relative_transform(/*this->nodes_manager_,*/ wheel_node,this->body_node_);
            cg::point_3 wheel_offset = wt.translation();
            wheel_offset.z = -wheel_offset.z; 

            //auto const *wnc = wheel_node->get_collision() ;
            //Assert(wnc) ;
            //cg::rectangle_3 bound = model_structure::bounding(*wnc);
            double radius = 0.75 * wheel_node->get_bound().radius;

            this->phys_vehicle_->add_wheel(30, /*bound.size().x / 2*/radius, /*bound.size().y/ 2*/radius, wheel_offset, wt.rotation().cpr(), true);

            this->wheels_.push_back(model::wheel_t(wheel_node));
        }
        return true;
    });
#else
    nm::visit_sub_tree(nodes_manager_->get_node_tree_iterator(0), [this](nm::node_info_ptr wheel_node)->bool
    {
        std::string name = wheel_node->name();
        if (boost::starts_with(wheel_node->name(), "wheel"))
        {
            cg::transform_4 wt = this->nodes_manager_->get_relative_transform(/*nodes_manager_,*/ wheel_node/*, this->body_node_*/);
            point_3 wheel_offset = wt.translation();
            FIXME(One more dirty trick)
            // Что тут за хрень с колесами прибавили, отняли нафига? 
            // Трансформ относительно body считается правильно, да он будет отрицательный по z
            // А вот дальше непонятный цирк 
            wheel_offset.z = abs(wheel_offset.z); 
            //auto const *wnc = wheel_node->get_collision() ;
            //Assert(wnc) ;
            //cg::rectangle_3 bound = model_structure::bounding(*wnc);
            //double radius = 0.75 * wheel_node->get_bound().radius;
            double radius = get_wheel_radius(wheel_node);//0.75 * (wheel_node->get_bound().size().z / 2.);

            this->phys_vehicle_->add_wheel(30, /*bound.size().x / 2*/radius, /*bound.size().y/ 2*/radius, wheel_offset, wt.rotation().cpr(), true);

            this->wheels_.push_back(model::wheel_t(wheel_node));
        }
        return true;
    });
#endif


    phys::ray_cast_vehicle::control_ptr(phys_vehicle_)->reset_suspension();
}

void model::go(cg::polar_point_2 const &offset)
{
    model_state_ = boost::make_shared<go_to_pos_state>(cg::geo_offset(pos(), offset), boost::none, !!aerotow_);
}

} // vehicle 


