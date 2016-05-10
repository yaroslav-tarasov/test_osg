#include "stdafx.h"


#include "ada/ada.h"
#include "phys/phys_sys_fwd.h"
#include "aircraft.h"
#include "phys_aircraft.h"
#include "phys/phys_sys.h"


namespace aircraft
{
#if 0
    bool fill_sensor(nodes_management::manager_ptr nodes_manager, phys::compound_sensor_t & s)
    {
        nm::node_info_ptr body_node   = nodes_manager->find_node("body");
        nm::node_info_ptr wing_r_node = nodes_manager->find_node("wing_r");
        nm::node_info_ptr wing_l_node = nodes_manager->find_node("wing_l");


        if (!body_node || !wing_r_node || !wing_l_node)
            return false;

        if (!body_node->get_collision())
            return false;

        phys::sensor_ptr body_s = phys::get_sensor(*body_node->get_collision());
        if (body_s->chunks_count() == 0)
            return false;

        s.add(cg::transform_4(), body_s);

        if (wing_l_node->get_collision())
        {
            phys::sensor_ptr wing_s = phys::get_sensor(*wing_l_node->get_collision());
            s.add(nodes_manager->get_relative_transform(/*nodes_manager,*/ wing_l_node, body_node), wing_s);
        }
        if (wing_r_node->get_collision())
        {
            phys::sensor_ptr wing_s = phys::get_sensor(*wing_r_node->get_collision());
            s.add(nodes_manager_->get_relative_transform(/*nodes_manager,*/ wing_r_node, body_node), wing_s);
        }

        nm::visit_sub_tree(nodes_manager->get_node_tree_iterator(body_node->node_id()), [&s, nodes_manager, body_node](nm::node_info_ptr n)->bool
        {
            if (boost::starts_with(n->name(), "engine"))
            {

                phys::sensor_ptr engine_s = phys::get_sensor(*n->get_collision());
                s.add(nodes_manager->get_relative_transform(/*nodes_manager,*/ n, body_node), engine_s);
            }
            return true;
        });

        return true;
    }
#endif

    phys_aircraft_ptr phys_aircraft_impl::create(cg::geo_base_3 const& base,
                                    phys::system_ptr phys, 
                                    //meteo::meteo_cursor_ptr meteo_cursor, 
                                    nodes_management::manager_ptr nodes_manager, 
                                    geo_position const& initial_position, 
                                    ada::data_t const& fsettings, 
                                    shassis_support_ptr shassis,
                                    size_t zone)
    {
        // phys::compound_sensor_t s;
        // if (!fill_sensor(nodes_manager, s))
        //     return phys_aircraft_ptr();
        phys::compound_sensor_ptr s = phys::aircraft::fill_cs(nodes_manager);

        return boost::make_shared<phys_aircraft_impl>(base, phys, /*meteo_cursor,*/ nodes_manager, initial_position, fsettings, shassis, s, zone);
    }

    phys_aircraft_impl::phys_aircraft_impl(geo_base_3 const& base, phys::system_ptr phys, /*meteo::meteo_cursor_ptr meteo_cursor,*/ nodes_management::manager_ptr nodes_manager, geo_position const& initial_position, ada::data_t const& fsettings, shassis_support_ptr shassis, phys::compound_sensor_ptr s, size_t zone)
        : base_(base)
        , zone_(zone)
        , phys_sys_(phys)
        , nodes_manager_(nodes_manager)
        , desired_position_(initial_position.pos)
        , desired_orien_(initial_position.orien)
        , shassis_(shassis)
        //, meteo_cursor_(meteo_cursor)
        , tow_attached_(false)
        , has_malfunction_(false)
        , prediction_(30.)
        , freeze_(true)
    {
        create_phys_aircraft(initial_position, fsettings, s);
    }

    phys_aircraft_impl::~phys_aircraft_impl()
    {
        // FIXME Testing needed 
        shassis_->visit_chassis([](shassis_group_t const&, shassis_t & shassis)
        {
            shassis.clear_wheels();
        });
    }

    void phys_aircraft_impl::update()
    {     
#if 0
        if (!freeze_)
        {
            shassis_->visit_chassis([this](aircraft::shassis_group_t const& gr, aircraft::shassis_t & shassis)
            {
                if(!gr.is_front)
                    return;

                auto wnode = shassis.wheel_node;

                nm::node_position np = wnode->position();
                np.local().orien = cg::quaternion(cg::cpr(this->phys_aircraft_->get_steer(),0,0));
                wnode->set_position(np);

                if (shassis.phys_wheels.empty())
                    return;

            });
        }
#endif 
        sync_phys(0.1);
    }

    void phys_aircraft_impl::attach_tow(bool attached)
    {
        tow_attached_ = attached;
    }

    void phys_aircraft_impl::freeze(bool freeze)
    {
        freeze_ = freeze; 
        if(freeze)
        {
            phys_aircraft_->set_thrust(0);                                 
            phys_aircraft_->set_brake(1); 
            phys_aircraft_->set_steer(0);
        }
    }

    void phys_aircraft_impl::go_to_pos(geo_point_3 const& pos, cg::quaternion const& orien)
    {
        desired_position_ = pos;
        desired_orien_ = orien;
    }
    
    void  phys_aircraft_impl::go_to_pos(geo_position const& pos)  
    {
        desired_position_ = pos.pos;
        desired_orien_ = pos.orien;
    }

    geo_position phys_aircraft_impl::get_position() const
    {
        //Assert(phys_aircraft_);

        decart_position body_pos = phys_aircraft_->get_position();
        decart_position root_pos = body_pos * body_transform_inv_; // Скорее всего здесь единица

        return geo_position(root_pos, base_);
    }

    decart_position phys_aircraft_impl::get_local_position() const
    {
        //Assert(phys_aircraft_);

        decart_position body_pos = phys_aircraft_->get_position();  
        decart_position root_pos = body_pos * body_transform_inv_; // Скорее всего здесь единица 
        
        return root_pos;
    }

    void phys_aircraft_impl::set_air_cfg(fms::air_config_t cfg)
    {
        cfg_ = cfg;
    }

    void phys_aircraft_impl::set_prediction(double prediction)
    {
        prediction_ = prediction;
    }

    geo_position phys_aircraft_impl::get_wheel_position( size_t i ) const
    {
        return geo_position(phys_aircraft_->get_wheel_position(i), base_);
    }

    phys::rigid_body_ptr phys_aircraft_impl::get_rigid_body() const
    {
        return phys_aircraft_;
    }

    void phys_aircraft_impl::set_steer   (double steer)
    {          
        phys_aircraft_->set_steer(steer);
    }

    void phys_aircraft_impl::set_brake   (double brake)
    {          
        phys_aircraft_->set_brake(brake);
    }

    double  phys_aircraft_impl::get_steer()
    {
        return  phys_aircraft_->get_steer();
    }

    std::vector<phys::aircraft::contact_info_t> phys_aircraft_impl::get_body_contacts() const
    {
        std::vector<phys::aircraft::contact_info_t> contacts = phys_aircraft_->get_body_contacts();
        for (auto it = contacts.begin(); it != contacts.end(); ++it)
            it->offset = it->offset * body_transform_inv_;

        return contacts;
    }

    bool phys_aircraft_impl::has_wheel_contact(size_t id) const
    {
        return phys_aircraft_->has_wheel_contact(id);
    }

    double phys_aircraft_impl::wheel_skid_info(size_t id) const
    {
        return phys_aircraft_->wheel_skid_info(id);
    }

    void phys_aircraft_impl::remove_wheel(size_t id)
    {
        phys_aircraft_->remove_wheel(id);
    }


    size_t phys_aircraft_impl::get_zone() const
    {
        return zone_;
    }

    void phys_aircraft_impl::set_malfunction(bool malfunction)
    {
        has_malfunction_ = malfunction;
    }


    void phys_aircraft_impl::create_phys_aircraft(geo_position const& initial_position, ada::data_t const& fsettings, phys::compound_sensor_ptr s)
    {
#ifndef SIMEX_MOD
        const double phys_mass_factor_ = 1; // 1000;  // 1; //  
#else
        const double phys_mass_factor_ = 1000;
#endif
        nm::node_info_ptr body_node = nodes_manager_->find_node("body");

        body_transform_inv_ = cg::transform_4(); 
        // FIXME TYV  сдается мне нефига не нужный код 
        // В модели симекса съедается трансформ на геометрии, и Body оказывается востребованным
        // get_relative_transform(nodes_manager_, body_node).inverted();
      
        FIXME(Осознать)
#if 0 // TODO or not TODO

        phys::collision_ptr collision = phys_sys_;

        double height = initial_position.pos.height;
        if (cg::eq_zero(initial_position.pos.height))
        {
            auto isection = collision->intersect_first(base_(geo_point_3(initial_position.pos, initial_position.pos.height + 10.)), 
                                                       base_(geo_point_3(initial_position.pos, initial_position.pos.height - 10.)));
            if (isection)
                height += 10 - 20. * *isection;
        }
#else
        double height = 0.0;
#endif

        transform_4 veh_transform = cg::transform_4(as_translation(base_(geo_point_3(initial_position.pos, height))), 
                                                    rotation_3(initial_position.orien.cpr())) * body_transform_inv_.inverted();

        decart_position p(veh_transform.translation(), initial_position.dpos, quaternion(veh_transform.rotation().cpr()), point_3());

        phys::aircraft::params_t params;
        double const mass = fsettings.max_mass / phys_mass_factor_;
        double const S = fsettings.S / phys_mass_factor_;
        double const min_speed = fsettings.v_stall_ld* fsettings.c_v_min;
        double const cd_0 = 2*(fsettings.cd_0_landing + fsettings.cd_0_landing_gear);
        double const cd_2 = fsettings.cd_2_landing;
        double const air_density = 1.225;
        double const g = 9.8;
        double const Cl = fsettings.max_mass * g / (air_density * fsettings.S * min_speed * min_speed);

        params.mass = fsettings.max_mass / phys_mass_factor_;
        params.S = S;
        params.wingspan = fsettings.span;
        params.chord = fsettings.S / params.wingspan;
        params.length = fsettings.length; 
        params.Cl = Cl;
        params.Cd0 = cd_0;
        params.Cd2 = cd_2;
        params.ClAOA = 0.4;
        params.Cs = 0.2;

#ifdef SIMEX_MOD
        params.thrust = (fsettings.ct_1 * (100. * 1000. / phys_mass_factor_ / fsettings.ct_2 + fsettings.ct_3 * 100. * 1000. / phys_mass_factor_ * 100. * 1000. / phys_mass_factor_ ));
#else        
        FIXME( "Не ну для разных двигателей разный, не все же реактивные" )
        const double hp = 0.0;
        const double ct_cr = 0.95; // Maximum cruise thrust coefficient
        params.thrust =  fsettings.ct_1 * (1 - hp/fsettings.ct_2 + fsettings.ct_3 * hp * hp) * ct_cr * 100;

        FIXME("И по какой формуле считать?")
        if(fsettings.engine == 1)
            params.thrust = 132050;
#endif

        phys_aircraft_ = phys_sys_->create_aircraft(params, s, p);
//        phys_aircraft_->set_control_manager(boost::bind(&phys_aircraft_impl::sync_phys, this, _1));

        double const wheel_mass = mass / 10;

#ifdef OSG_NODE_IMPL 
        shassis_->visit_chassis([this, wheel_mass, &body_node,&s](shassis_group_t const& group, shassis_t & shassis)
        {
            auto node = shassis.wheel_node;

            if (!group.opened)
                return;

            double mass = wheel_mass;
            size_t fake_count = 0;

            //if (group.is_front)
            //    mass = wheel_mass*10, fake_count = 2;

            //cg::transform_4 wt = nm::get_relative_transform(this->nodes_manager_, node, body_node);
            cg::transform_4 wt = this->nodes_manager_->get_relative_transform(/*this->nodes_manager_,*/ node);
            point_3 wheel_offset = wt.translation() /*+ s->get_offset()*/;

            //cg::rectangle_3 bound = model_structure::bounding(*node->get_collision());
            const double radius = 0.75 * node->get_bound().radius ;

            size_t wid = phys_aircraft_->add_wheel(/*mass*/0.f, /*bound.size().x / 2.*/0.f, /*0.75 * (bound.size().y / 2.)*/radius, /*wt.translation()*/wheel_offset, wt.rotation().cpr(), true, group.is_front);
            shassis.phys_wheels.push_back(wid);

            for (size_t j = 0; j < fake_count; ++j)
            {
                size_t wid = phys_aircraft_->add_wheel(/*mass*/0.f, /*bound.size().x / 2.*/0.f, /*0.75 * (bound.size().y / 2.)*/radius, /*wt.translation()*/wheel_offset, wt.rotation().cpr(), true, group.is_front);
                shassis.phys_wheels.push_back(wid);
            }
        });
#else
        shassis_->visit_chassis([this, wheel_mass, &body_node,&s](shassis_group_t const& group, shassis_t & shassis)
        {
            auto node = shassis.wheel_node;

            if (!group.opened)
                return;

            double mass = wheel_mass;
            size_t fake_count = 1;

            if (group.is_front)
                mass = wheel_mass*10, fake_count = 2;

            cg::transform_4 wt = this->nodes_manager_->get_relative_transform(/*this->nodes_manager_,*/ node/*,body_node*/);
            point_3 wheel_offset = wt.translation() + s->get_offset() + cg::point_3(0.,0.,group.is_front?0.0:-(0.1));

            //cg::rectangle_3 bound = model_structure::bounding(*node->get_collision());
            //const double radius = 0.75 * node->get_bound().radius ;
            double radius = get_wheel_radius(node);//0.75 * (node->get_bound().size().z / 2.);
            
            size_t wid = phys_aircraft_->add_wheel(/*mass*/0.f, /*bound.size().x / 2.*/0.f, /*0.75 * (bound.size().y / 2.)*/radius, /*wt.translation()*/wheel_offset, wt.rotation().cpr(), true, group.is_front);
            shassis.phys_wheels.push_back(wid);

            for (size_t j = 0; j < fake_count; ++j)
            {
                size_t wid = phys_aircraft_->add_wheel(/*mass*/0.f, /*bound.size().x / 2.*/0.f, /*0.75 * (bound.size().y / 2.)*/radius, /*wt.translation()*/wheel_offset, wt.rotation().cpr(), true, group.is_front);
                shassis.phys_wheels.push_back(wid);
            }
        });
#endif

        phys_aircraft_->reset_suspension();
    }

    void phys_aircraft_impl::sync_phys(double dt)
    {
        if (!phys_aircraft_)
            return;

        decart_position cur_pos = phys_aircraft_->get_position();
        decart_position root_pos = cur_pos * body_transform_inv_;
        geo_position cur_glb_pos(cur_pos, base_);
        geo_position root_glb_pos(root_pos, base_);

        point_3 wind(0.0,0.0,0.0);
#if 0
        if (meteo_cursor_)
        {
            meteo_cursor_->move_to(cur_glb_pos.pos, 0);
            wind = meteo_cursor_->wind();
        }
#endif

        //LogTrace("phys height " << cur_pos.pos.z);

        if ( tow_attached_ )
        {
            //phys_aircraft_->set_steer(steer);
            phys_aircraft_->set_thrust(0);
            //phys_aircraft_->set_brake(0);
            phys_aircraft_->set_elevator(0);
            phys_aircraft_->set_ailerons(0);

            // TODO  // by simex 
            //send(msg::phys_pos_msg(root_glb_pos.pos, root_glb_pos.orien.get_course()));

            return;
        }

        double const air_density = 1.225;

        double cur_speed = cg::norm(cur_pos.dpos);

        point_2 loc_cur_dpos = point_2(cur_pos.dpos) * cg::rotation_2(cur_pos.orien.get_course());

        double cur_speed_signed = loc_cur_dpos.y < 0 ? -cur_speed : cur_speed;

        double cur_course = cur_pos.orien.cpr().course;
        double cur_roll = cur_pos.orien.cpr().roll;

        point_3 forward_dir = cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 1, 0))) ;
        point_3 right_dir   = cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(1, 0, 0))) ;
        point_3 up_dir      = cg::normalized_safe(cur_pos.orien.rotate_vector(point_3(0, 0, 1))) ;

        point_3 vk = cur_pos.dpos - wind;

        point_3 Y = !cg::eq_zero(cg::norm(vk)) ? cg::normalized(vk) : forward_dir;
        point_3 Z = cg::normalized_safe(right_dir ^ Y);
        point_3 X = cg::normalized_safe(Y ^ Z);
        cg::rotation_3 vel_rotation(X, Y, Z);

        cg::rotation_3 rot(desired_orien_.cpr());

//        geo_base_3 tgt_pos = geo_base_3(desired_position_)(rot * body_transform_inv_.inverted().translation());

//        geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*dt));

        geo_base_3 predict_pos = desired_position_;
        geo_base_3 predict_tgt_pos = predict_pos(rot * body_transform_inv_.inverted().translation());


        if (predict_tgt_pos.height < 0)
            predict_tgt_pos.height = 0;

        double dist2target = cg::distance2d(cur_glb_pos.pos, predict_tgt_pos);
        point_2 offset = cg::point_2(cur_glb_pos.pos(predict_tgt_pos));

        point_3 Y_right_dir_proj =  Y - Y * right_dir * right_dir;
        double attack_angle = cg::rad2grad(cg::angle(Y_right_dir_proj, forward_dir)) * (-cg::sign(Y * up_dir));
        double slide_angle = cg::rad2grad(cg::angle(Y, Y_right_dir_proj))  * (-cg::sign(Y * right_dir));
        double cos_a = cos(cg::grad2rad(attack_angle));
        double sin_a = sin(cg::grad2rad(attack_angle));
        double cos_b = cos(cg::grad2rad(slide_angle));
        double sin_b = sin(cg::grad2rad(slide_angle));

        double const q = 0.5 * air_density * cg::sqr(cg::norm(cur_pos.dpos - wind)); // dynamic pressure
        double const wingspan = phys_aircraft_->params().wingspan;
        double const chord = phys_aircraft_->params().chord;
        double const S = phys_aircraft_->params().S;

        //geo_base_3 steer_predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(steer_prediction*dt));
//         geo_base_3 steer_predict_pos = desired_position_;
// 
//         geo_base_3 steer_predict_tgt_pos = steer_predict_pos(rot * body_transform_inv_.inverted().translation());
//         point_2 loc_offset = offset * cg::rotation_2(cur_glb_pos.orien.get_course());
//         double dist2target_signed = loc_offset.y < 0 ? -dist2target : dist2target;
//         double desired_course = cg::polar_point_2(cur_glb_pos.pos(steer_predict_tgt_pos)).course;
//         if (dist2target_signed < 0)
//             desired_course = cg::norm180(180 + desired_course);

        double steer = 0;

        double thrust = 0;
        double elevator = 0;
        double ailerons = 0;
        double brake = 0;
        double rudder = 0;
        
        const double  min_aerodynamic_speed = phys_aircraft_->params().min_aerodynamic_speed;
        
        if (cur_speed < min_aerodynamic_speed )
        {
            on_ground_ = true;

            //geo_base_3 steer_predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(steer_prediction*dt));
            geo_base_3 steer_predict_pos = desired_position_;

            geo_base_3 steer_predict_tgt_pos = steer_predict_pos(rot * body_transform_inv_.inverted().translation());
            point_2 loc_offset = offset * cg::rotation_2(cur_glb_pos.orien.get_course());
            double dist2target_signed = loc_offset.y < 0 ? -dist2target : dist2target;
            double desired_course = cg::polar_point_2(cur_glb_pos.pos(steer_predict_tgt_pos)).course;
            if (dist2target_signed < 0)
                desired_course = cg::norm180(180 + desired_course);

            steer = cg::bound(cg::norm180(desired_course - cur_course),-65., 65.);


            //double max_speed_clamped = cur_speed < 70 ? cg::clamp(1., 2., max_speed, 5.)(fabs(steer)) : max_speed;
            //double max_speed_clamped = max_speed;

            double desired_speed_signed = (dist2target_signed - cur_speed_signed) / (1.2 * prediction_*dt);
            
            {
                // force_log fl;

                LOG_ODS_MSG(
                    "desired_speed_signed:  "                << desired_speed_signed << "\n" 
                    );
            }

// FIXME TYV      А не то при рулении взлетаем хвостом вперед хо-хо   
            // desired_speed_signed = cg::bound(desired_speed_signed, -30., 30.);

            //        double desired_speed_signed = filter::BreakApproachSpeed(0., dist2target_signed, cur_speed_signed, max_speed_clamped, 500, dt, prediction);
            if (fabs(desired_speed_signed) < 0.1)
                desired_speed_signed = 0;

            if (!has_malfunction_)
            {
                thrust = phys_aircraft_->params().mass * (desired_speed_signed - cur_speed_signed) / (1.1 * dt);
                thrust /= phys_aircraft_->params().thrust;
                thrust = cg::bound(thrust, -1., 1.);
            }

            if (has_malfunction_)
                brake = 1;

            //LogTrace(" thrust " << thrust << " attack_angle " << attack_angle << " slide_angle " << slide_angle);
        }
        else
        {

            double desired_slide_angle  = slide_angle;
            double desired_thrust       = phys_aircraft_->thrust();
            double desired_attack_angle = attack_angle;  // TYV 0

            point_3 desired_vel = geo_base_3(cur_glb_pos.pos)(predict_tgt_pos) / (1.2 * prediction_ * dt);
            
            if (cfg_ == fms::CFG_GD)
                desired_vel.z = 0;

            point_3 desired_accel((desired_vel - cur_pos.dpos) / 0.3, 
                                  (desired_vel.z - cur_pos.dpos.z) / 0.2);

            point_3 desired_accel_xz =  desired_accel - desired_accel*Y*Y;
            point_3 desired_accel_xz_velocity = desired_accel_xz * vel_rotation;

            bool reverse = false;
            bool low_attack = false;

            if (cfg_ == fms::CFG_GD/* && root_glb_pos.pos.height < 0.5*/)
            {
                reverse = true;
                if (on_ground_)
                    low_attack = true;
            }

            //         bool low_attack = false;
            //         if (root_glb_pos.pos.height < 5)
            //             low_attack = true;

            calc_phys_controls(desired_slide_angle, desired_thrust, desired_attack_angle, q, vel_rotation, desired_accel, wind, reverse, low_attack);

            double delta_roll = cg::polar_point_2(point_2(desired_accel_xz_velocity.x, desired_accel_xz_velocity.z)).course;
            delta_roll = cg::clamp3(0., 5., 400., -cur_roll, 0., delta_roll)(cg::norm(desired_accel_xz_velocity));
            //         if (fabs(delta_roll) > 90)
            //         {
            //             delta_roll = cg::norm180(180 + delta_roll);
            //         }

            double max_roll = 15;
            max_roll = cg::clamp(5., 30., 0., max_roll)(cur_glb_pos.pos.height);
            
            {
                // force_log fl;

                LOG_ODS_MSG(
                    "max_roll:  "                << max_roll << "\n" <<
                    "desired_slide_angle:  "                << desired_slide_angle << "\n" <<
                    "desired_thrust:  "                << desired_thrust << "\n" <<
                    "desired_attack_angle:  "                << desired_attack_angle << "\n" 
                    );
            }

            double roll_omega_smooth  = on_ground_ ? 2. : 0.3;
            double aa_omega_smooth    = on_ground_ ? 2. : 0.2;
            double slide_omega_smooth = on_ground_ ? 2. : 0.5;

            double desired_roll = cg::bound(cur_roll + delta_roll, -max_roll, max_roll);
            double desired_droll = (desired_roll - cur_pos.orien.get_roll()) / roll_omega_smooth;

            //         if (cg::norm(desired_accel_xz_velocity) < 60)  
            //             desired_droll += cg::clamp(0., 60., 0., -cur_roll / 0.6)(cg::norm(desired_accel_xz_velocity));

            point_3 desired_omega; 
            desired_omega += (desired_attack_angle - attack_angle) / aa_omega_smooth * (cur_pos.orien.rotate_vector(point_3(cos_b, sin_b, 0)));
            desired_omega += -(desired_slide_angle - slide_angle) / slide_omega_smooth * (cur_pos.orien.rotate_vector(point_3(-sin_a*sin_b, sin_a*cos_b, cos_a)));
            desired_omega += desired_droll * forward_dir;

            point_3 desired_omega_loc = (!cur_pos.orien).rotate_vector(desired_omega);
            point_3 cur_loc_omega     = (!cur_pos.orien).rotate_vector(cur_pos.omega);

            point_3 desired_domega = (desired_omega_loc - cur_loc_omega) / (dt);

            elevator = phys_aircraft_->Ixx() * desired_domega.x / (phys_aircraft_->params().elevator * q * chord * S);
            ailerons = phys_aircraft_->Iyy() * desired_domega.y / (phys_aircraft_->params().ailerons * q * wingspan * S);
            rudder   = phys_aircraft_->Izz() * desired_domega.z / (phys_aircraft_->params().rudder * q * wingspan * S);

            elevator = cg::bound(elevator, -1., 1.);
            ailerons = cg::bound(ailerons, -1., 1.);
            rudder   = cg::bound(rudder, -1., 1.);

            thrust   = cg::bound(desired_thrust, -1., 1.);

            //LogTrace("slide_angle " << slide_angle << " thrust " << thrust << " aa " << attack_angle << " desired_attack_angle " << desired_attack_angle << " desired_accel.z " << desired_accel.z );
        }   


        phys_aircraft_->set_steer(steer);
        phys_aircraft_->set_thrust(thrust);
        phys_aircraft_->set_brake(brake);
        phys_aircraft_->set_elevator(elevator);
        phys_aircraft_->set_ailerons(ailerons);
        phys_aircraft_->set_rudder(rudder);
        phys_aircraft_->set_wind(wind);


       // TODO
       //         send(msg::phys_pos_msg(root_glb_pos.pos, root_glb_pos.orien.get_course()));
    }

    void phys_aircraft_impl::calc_phys_controls(double & slide_angle, double & thrust, double & attack_angle, double q, cg::rotation_3 const& vel_rotation, cg::point_3 const& desired_accel, cg::point_3 const& /*wind*/, bool reverse, bool low_attack)
    {
        double x[4]; //(b, t, a, c)
        x[0] = slide_angle;
        x[1] = thrust;
        x[2] = attack_angle;
        x[3] = 0;

        struct func1_t
        {
            func1_t( phys::aircraft::params_t const& params, double q, point_3 const& accel )
                : params_(params)
                , q_(q)
                , accel_(accel)
            {}

            cg::point_3 calculate( double x[4] )
            {
                double cos_a = cos(cg::grad2rad(x[2])), sin_a = sin(cg::grad2rad(x[2])), cos_b = cos(cg::grad2rad(x[0])), sin_b = sin(cg::grad2rad(x[0]));
                //            double cos_c = cos(cg::grad2rad(x[3])), sin_c = sin(cg::grad2rad(x[3]));

                double lift = params_.Cl * params_.S * q_;
                double liftAOA = params_.ClAOA * params_.S * q_;
                double drag = (params_.Cd0 + params_.Cd2 * cg::sqr(params_.Cl) )* params_.S * q_;
                double slide = params_.Cs * params_.S * q_;
                double thrust = params_.thrust;

                point_3 f;
                f.x = thrust * x[1]*(sin_b) + slide * x[0]              + params_.mass*accel_.x;
                f.y = thrust * x[1]*(cos_a*cos_b) - drag                + params_.mass*accel_.y;
                f.z = thrust * x[1]*(sin_a*cos_b) + lift + liftAOA*(x[2]+params_.aa0) + params_.mass*accel_.z;
                return f;
            }

        private:
            phys::aircraft::params_t params_;
            double q_;
            cg::point_3 accel_;
        };

        struct func2_t
        {
            func2_t( double kx, double ky, double kz )
                : kx(kx), ky(ky), kz(kz)
            {}

            double calculate( point_3 const& f, double x[4] )
            {
                (void *)x;
                return kx * f.x * f.x + ky * f.y * f.y + kz * f.z * f.z;
            }

        private:
            double kx, ky, kz;
        };


        point_3 g(0,0.,-9.8);
        point_3 g_vel = g * vel_rotation;
        point_3 desired_accel_vel = desired_accel * vel_rotation;

        double const kx = 1., ky = 1., kz = 1;

        point_3 free_forces = -desired_accel_vel;
        if (!reverse)
            free_forces += g_vel;

        func1_t fn(phys_aircraft_->params(), q, free_forces);
        std::function<cg::point_3(double[4])> f = std::bind(&func1_t::calculate, &fn , sp::_1);
        std::function<double(point_3, double[4])> f2 = std::bind(&func2_t::calculate, func2_t(kx, ky, kz), sp::_1, sp::_2);

        //    double lift = phys_aircraft_->params().Cl * phys_aircraft_->params().S * q;
        double liftAOA = phys_aircraft_->params().ClAOA * phys_aircraft_->params().S * q;
        //    double drag = phys_aircraft_->params().Cd * phys_aircraft_->params().S * q;
        double slide = phys_aircraft_->params().Cs * phys_aircraft_->params().S * q;
        double thrust_coeff = phys_aircraft_->params().thrust;

        //////////////////////////////////////////////////////////////////////////

        size_t i = 0;
        double J[4][4];
        i = 0;
        for (i = 0; i < 10; ++i)
        {
            double cos_a = cos(cg::grad2rad(x[2])), sin_a = sin(cg::grad2rad(x[2])), cos_b = cos(cg::grad2rad(x[0])), sin_b = sin(cg::grad2rad(x[0]));
            //        double cos_c = cos(cg::grad2rad(x[3])), sin_c = sin(cg::grad2rad(x[3]));

            point_3 ff = f(x);

            // derivative b
            J[0][0] = thrust_coeff * x[1]*(cos_b) + slide;
            J[1][0] = thrust_coeff * x[1]*(-cos_a*sin_b);
            J[2][0] = thrust_coeff * x[1]*(-sin_a*sin_b);
            J[3][0] = 0;

            // derivative t
            J[0][1] = thrust_coeff * (sin_b);
            J[1][1] = thrust_coeff * (cos_a*cos_b);
            J[2][1] = thrust_coeff * (sin_a*cos_b);
            J[3][1] = 0;

            // derivative a
            J[0][2] = 0;
            J[1][2] = thrust_coeff * x[1] * (-sin_a*cos_b);
            J[2][2] = thrust_coeff * x[1] * (cos_a*cos_b) + liftAOA;
            J[3][2] = 0;

            // derivative c
            J[0][3] = 0;
            J[1][3] = 0;
            J[2][3] = 0;
            J[3][3] = 0;

            double derivative[4] ;

            double derivative_len_sqr = 0;
            for (size_t j = 0; j < 4; ++j)
            {
                derivative[j] = kx * 2 * ff.x * J[0][j] + ky * 2 * ff.y * J[1][j] + kz * 2 * ff.z * J[2][j] + J[3][j];
                derivative_len_sqr += derivative[j]*derivative[j];
            }
            double derivative_len = cg::sqrt(derivative_len_sqr);
            if (cg::eq_zero(derivative_len))
                break;
            for (size_t j = 0; j < 4; ++j)
                derivative[j] /= -derivative_len;

            cg::range_2 step_max_range[4];
            step_max_range[0] = cg::range_2(-100., 100.);
            step_max_range[1] = cg::range_2(reverse ? -1.: 0., 1.);
            step_max_range[2] = cg::range_2(low_attack ? -1 : -15., low_attack ? 0. : +15.);
            step_max_range[3] = cg::range_2(-90., 90.);

            double l = 0;
            double m = 100.;
            for (size_t j = 0; j < 4; ++j)
            {
                if (!cg::eq_zero(derivative[j]))
                {
                    double mm;
                    if (derivative[j] >= 0)
                        mm = cg::bound(m, (step_max_range[j].lo() - x[j]) / derivative[j], (step_max_range[j].hi() - x[j]) / derivative[j]);
                    else
                        mm = cg::bound(m, (step_max_range[j].hi() - x[j]) / derivative[j], (step_max_range[j].lo() - x[j]) / derivative[j]);

                    if (cg::eq_zero(mm))
                        derivative[j] = 0;
                    else 
                        m = mm;
                }
            }

            double f_l = f2(f(x), x);
            double x_m[4];
            for (size_t j = 0; j < 4; ++j)
                x_m[j] = x[j] + derivative[j]*m;
            double f_m = f2(f(x_m), x_m);
            for (size_t k = 0; k < 10; ++k)
            {
                double n = (m+l)*0.5;
                double x_n[4];
                for (size_t j = 0; j < 4; ++j)
                    x_n[j] = x[j] + derivative[j]*n;
                //            double f_n = f2(f(x_n), x_n);

                double ll = (l+n)*0.5;
                double x_ll[4];
                for (size_t j = 0; j < 4; ++j)
                    x_ll[j] = x[j] + derivative[j]*ll;
                double f_ll = f2(f(x_ll), x_ll);

                double mm = (m+n)*0.5;
                double x_mm[4];
                for (size_t j = 0; j < 4; ++j)
                    x_mm[j] = x[j] + derivative[j]*mm;
                double f_mm = f2(f(x_mm), x_mm);

                if (f_ll < f_mm)
                    m = mm, f_m = f_mm;
                else 
                    l = ll, f_l = f_ll;
            }

            for (size_t j = 0; j < 4; ++j)
                x[j] = step_max_range[j].closest_point(x[j] + derivative[j] * m);

            x[0] = cg::norm180(x[0]);
        }

        //////////////////////////////////////////////////////////////////////////


        slide_angle = x[0];
        attack_angle = x[2];
        //double desired_delta_roll_angle = x[3];
        thrust = x[1];                                                                      
    }

}

