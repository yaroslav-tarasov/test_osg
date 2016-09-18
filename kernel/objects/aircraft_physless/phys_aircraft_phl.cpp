#include "stdafx.h"

#include "ada/ada.h"
#include "phys/phys_sys_fwd.h"
#include "aircraft.h"
#include "phys_aircraft_phl.h"
#include "phys/phys_sys.h"


//using namespace cg;

namespace aircraft_physless
{

    aircraft::phys_aircraft_ptr phys_aircraft_impl::create(cg::geo_base_3 const& base,
                                    phys::system_ptr phys, 
                                    //meteo::meteo_cursor_ptr meteo_cursor, 
                                    nodes_management::manager_ptr nodes_manager, 
                                    geo_position const& initial_position, 
                                    ada::data_t const& fsettings, 
                                    shassis_support_ptr shassis,
                                    size_t zone)
    {
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
        sync_phys(/*0.1*/cfg().model_params.msys_step);
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
        desired_geo_position_ = pos;
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
        double height = initial_position.pos.height;
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

        phys_aircraft_ = phys_sys_->create_aircraft_pl(params, s, p);
//        phys_aircraft_->set_control_manager(boost::bind(&phys_aircraft_impl::sync_phys, this, _1));

        double const wheel_mass = mass / 10;

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

        phys_aircraft_->reset_suspension();
    }

    void phys_aircraft_impl::sync_phys(double /*dt*/)
    {
        if (!phys_aircraft_)
            return;
		
		decart_position cur_pos = phys_aircraft_->get_position();
		decart_position root_pos = cur_pos * body_transform_inv_;
		geo_position cur_glb_pos(cur_pos, base_);
		geo_position root_glb_pos(root_pos, base_);

        if ( tow_attached_ )
        {
            //phys_aircraft_->set_steer(steer);
            phys_aircraft_->set_thrust(0);
            //phys_aircraft_->set_brake(0);
            phys_aircraft_->set_elevator(0);
            phys_aircraft_->set_ailerons(0);

			FIXME("Наверное зависит от направления буксировки, причем сильно")
            //return;
        }

        double steer = 0;
        double thrust = 0;
        double elevator = 0;
        double ailerons = 0;
        double brake = 0;
        double rudder = 0;
        
        const double  min_aerodynamic_speed = phys_aircraft_->params().min_aerodynamic_speed;
        

        elevator = cg::bound(elevator, -1., 1.);
        ailerons = cg::bound(ailerons, -1., 1.);
        rudder   = cg::bound  (rudder, -1., 1.);
        thrust   = cg::bound  (thrust, -1., 1.);

        FIXME(Set steer);
        //phys_aircraft_->set_steer(steer);
        phys_aircraft_->set_thrust(thrust);
        phys_aircraft_->set_brake(brake);
        phys_aircraft_->set_elevator(elevator);
        phys_aircraft_->set_ailerons(ailerons);
        phys_aircraft_->set_rudder(rudder);
        // phys_aircraft_->set_wind(wind);

		phys_aircraft_->set_position(decart_position(base_(desired_position_),desired_geo_position_.dpos,desired_orien_,point_3f()));
    }

}

