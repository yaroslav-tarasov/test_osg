#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_physless_model.h"
#include "aircraft_physless_common.h"

#include "sync_fsm/sync_pl_phys_state.h"
#include "sync_fsm/sync_pl_none_state.h"
#include "sync_fsm/sync_pl_fms_state.h"
//

#include "aircraft\aircraft_shassis_impl.h"
#include "aircraft\aircraft_rotors_impl.h"


namespace aircraft_physless
{
     
    const double model::shassi_height_ = 130;


object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc, dict));
}

AUTO_REG_NAME(aircraft_physless_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict )
    : view(oc,dict)
    , sys_(dynamic_cast<model_system *>(oc.sys))
    , airports_manager_(find_first_object<airports_manager::info_ptr>(collection_))
    , fast_session_    (false)
    , nm_ang_smooth_   (2)
    , rotors_angular_speed_ (0)
{

    if (get_nodes_manager())
    {
        root_               = get_nodes_manager()->get_node(0);
        elev_rudder_node_   = get_nodes_manager()->find_node("elevrudr");
        rudder_node_        = get_nodes_manager()->find_node("rudder");
        tow_point_node_     = get_nodes_manager()->find_node("tow_point");
        body_node_          = get_nodes_manager()->find_node("body");
    }

    shassis_ = boost::make_shared<aircraft::shassis_support_impl>(get_nodes_manager());
    rotors_ =  boost::make_shared<aircraft::rotors_support_impl>(get_nodes_manager());
    sync_state_.reset(new sync_fsm::none_state(*this));

     conn_holder() << dynamic_cast<system_session *>(sys_)->subscribe_time_factor_changed(boost::bind(&model::on_time_factor_changed, this, _1, _2));

}

void model::update( double time )
{   
    view::update(time);
    update_len(time);
    
    //if (!fpl_ || fpl_->active())
    //    aircraft_fms::model_control_ptr(get_fms_info())->activate();
        
    FIXME(init_shassi_anim)
    init_shassi_anim();

    double dt = time - (last_update_ ? *last_update_ : 0);
    if (cg::eq_zero(dt))
        return;

    desired_nm_pos_.reset();
    desired_nm_orien_.reset();

    sync_fsm::state_ptr prev_state = sync_state_;
    sync_state_->update(time, dt);

    sync_nm_root(dt);

    update_contact_effects(time);
    check_wheel_brake();
    FIXME(update_shassi_anim)
    update_shassi_anim(time);
    //update_atc_state();
    sync_fms();

    check_rotors_malfunction();

    //if (phys_aircraft_)
    //{
    //    bool has_malfunction = false;
    //    if (malfunction(MF_CHASSIS_FRONT) || malfunction(MF_CHASSIS_REAR_LEFT) || malfunction(MF_CHASSIS_REAR_RIGHT))
    //        has_malfunction = true;

    //    phys_aircraft_->set_malfunction(has_malfunction);
    //}

    last_update_ = time;
}

void model::sync_nm_root(double /*dt*/)
{
    Assert(root_);

    if (!desired_nm_pos_ || !desired_nm_orien_)
        return;

    geo_point_3 const desired_pos   = *desired_nm_pos_;
    quaternion  const desired_orien = *desired_nm_orien_;
    
    LOG_ODS_MSG( "sync_nm_root:  desired_pos:  x:  "  << desired_pos.lat << "    y: " << desired_pos.lon << "\n" );

    nodes_management::node_position root_node_pos = root_->position();
    // 
    FIXME(Если раскоментарить отлично синхронизируемся)
    //root_node_pos.global().pos = desired_pos;
    
    root_node_pos.global().dpos = geo_base_3(root_node_pos.global().pos)(desired_pos) / (sys_->calc_step());
    root_node_pos.global().omega = cg::get_rotate_quaternion(root_node_pos.global().orien, desired_orien).rot_axis().omega() / (nm_ang_smooth_ * sys_->calc_step());
    
    LOG_ODS_MSG( "sync_nm_root:   root_node_pos.global().pos :   x:  "  <<  root_node_pos.global().pos.lat << "    y: " << root_node_pos.global().pos.lon <<  "  calc_step=" << sys_->calc_step() << "\n" );
    LOG_ODS_MSG( "sync_nm_root:   root_node_pos.global().dpos :  x:  "  <<  root_node_pos.global().dpos.x << "    y: " << root_node_pos.global().dpos.y << "\n" );

    root_->set_position(root_node_pos);
}


airports_manager::info_ptr model::get_airports_manager() const
{
    return airports_manager_;
}

//phys::control_ptr model::phys_control() const
//{
//    return phys_;
//}

nodes_management::manager_ptr model::get_nodes_manager() const
{
    return view::get_nodes_manager();
}


aircraft::shassis_support_ptr model::get_shassis() const
{
    return shassis_;
}

aircraft::rotors_support_ptr model::get_rotors() const
{
    return rotors_;
}

fms::trajectory_ptr model::get_trajectory() const 
{
    return traj_;
}

geo_position model::get_root_pos() const
{
    return root_->position().global();
}

bool model::is_fast_session() const
{
    return fast_session_;
}

void model::set_desired_nm_pos  (geo_point_3 const& pos)
{
    desired_nm_pos_ = pos;
    
    LOG_ODS_MSG( "set_desired_nm_pos:  x:  "  << pos.lat << "    y: " << pos.lon << "\n" );

}

void model::set_desired_nm_orien(quaternion const& orien)
{
    desired_nm_orien_ = orien;
}

void model::on_malfunction_changed( aircraft::malfunction_kind_t kind ) 
{
    if (kind == aircraft::MF_CHASSIS_FRONT)
    {
        shassis_->set_malfunction(aircraft::SG_FRONT, malfunction(kind));
    }
    else if (kind == aircraft::MF_CHASSIS_REAR_LEFT)
    {
        shassis_->set_malfunction(aircraft::SG_REAR_LEFT, malfunction(kind));
    }
    else if (kind == aircraft::MF_CHASSIS_REAR_RIGHT)
    {
        shassis_->set_malfunction(aircraft::SG_REAR_RIGHT, malfunction(kind));
    }
    else if (kind == aircraft::MF_ONE_ENGINE || kind == aircraft::MF_ALL_ENGINES)
    {
        double factor = 1;
        if (malfunction(aircraft::MF_ALL_ENGINES))
            factor = 0;
        else if (malfunction(aircraft::MF_ONE_ENGINE))
        {
            factor = 0.7;
            FIXME(Test)
            rotors_->set_malfunction(aircraft::RG_REAR_LEFT,malfunction(kind));
        }
        // FIXME TODO OR NOT TODO
        //auto controls = flight_manager_control_->get_controls();
        //controls.engine_factor = factor;
        //flight_manager_control_->set_controls(controls);
    }
    else if (kind == aircraft::MF_FUEL_LEAKING)
    {   
        // FIXME TODO OR NOT TODO
        //auto controls = flight_manager_control_->get_controls();
        //controls.fuel_leaking = true;
        //flight_manager_control_->set_controls(controls);
    }
}

// TYV
void model::on_new_contact_effect(double /*time*/, std::vector<contact_t> const& contacts)
{
    FIXME(Критерий сильного удара?);

    float vel = 0;
    for (size_t i = 0; i < contacts.size(); ++i)
    {
          vel = cg::max(cg::norm(contacts[i].vel), vel);
    }

     if(traj_  && vel>50)
         traj_.reset();
}

void model::init_shassi_anim ()
{
    if (shassi_anim_inited_)
        return;

    if (root_ /*&& get_fms_info()*/)
    {
        geo_point_3 const& pos =/* get_fms_info()->*/get_state().dyn_state.pos;

        bool to_be_opened = pos.height < shassi_height_;

        shassis_->visit_groups([to_be_opened](aircraft::shassis_group_t & shassis_group)
        {
            if (to_be_opened)
                shassis_group.open(true);
            else
                shassis_group.close(true);
        });
    }

    shassi_anim_inited_ = true;
}

void model::update_shassi_anim (double time)
{
    if (last_shassi_play_ && time < *last_shassi_play_ + 5)
        return;

    if (root_ /*&& get_fms_info()*/)
    {
        geo_point_3 const& pos = /*get_fms_info()->*/get_state().dyn_state.pos;

        bool to_be_opened = pos.height < shassi_height_;

        shassis_->visit_groups([this, to_be_opened, time](aircraft::shassis_group_t & shassis_group)
        {
            if (!shassis_group.malfunction)
            {
                if (shassis_group.opened && !to_be_opened)
                {
                    shassis_group.close();
                    this->last_shassi_play_ = time;
                }
                else if (!shassis_group.opened && to_be_opened)
                {
                    shassis_group.open();
                    this->last_shassi_play_ = time;
                }
            }
        });
    }
}

void model::update_contact_effects(double time)
{
#if 0

    if (!phys_aircraft_)
        return;

    auto contacts = std::move(phys_aircraft_->get_body_contacts());
    if (!contacts.empty())
    {
        vector<msg::contact_effect::contact_t> contacts_to_send;

        for (auto it = contacts.begin(); it != contacts.end(); ++it)
            contacts_to_send.push_back(msg::contact_effect::contact_t(it->offset, it->vel));

        set(msg::contact_effect(move(contacts_to_send), time), false);
    }

    shassis_->visit_chassis([this, time](shassis_group_t const&, shassis_t& shassis)
    {
        bool has_contact = false;
        for (size_t  i = 0 ; i < shassis.phys_wheels.size(); ++i)
            if (this->phys_aircraft_->has_wheel_contact(shassis.phys_wheels[i]))
                has_contact = true;

        if (has_contact)
        {
            //             double skid = this->phys_aircraft_->wheel_skid_info(shassis.phys_wheels[0]);
            if (!shassis.landing_dust)
            {
                geo_position wpos = this->phys_aircraft_->get_wheel_position(shassis.phys_wheels[0]);
                geo_position body_pos = this->phys_aircraft_->get_position();
                cg::point_3 loc_omega = (!body_pos.orien).rotate_vector(wpos.omega);
                cg::point_3 vel = body_pos.dpos - body_pos.orien.rotate_vector(cg::point_3(loc_omega.x,0,0) * cg::grad2rad()) * shassis.radius;

                if (cg::norm(vel) > 50)
                {
                    point_3 loc_offset = (!body_pos.orien).rotate_vector(body_pos.pos(wpos.pos));
                    loc_offset += cg::point_3(0,0,shassis.radius);
                    this->set(msg::wheel_contact_effect(time, loc_offset, vel), false);

                    shassis.landing_dust = true;
                }
            }
        }
        else
        {
            shassis.landing_dust = false;
        }

    });     

#endif

}



void model::sync_fms(bool force)
{
#if 1	
  //  if (!phys_aircraft_)
		//return ;

	geo_position fmspos = fms_pos();
	nodes_management::node_position root_node_pos = root_->position();
    geo_position physpos = geo_position(root_node_pos.global().pos,root_node_pos.global().orien); // phys_aircraft_->get_position();

	double prediction = 300;
    _state.dyn_state.pos = physpos.pos;
    _state.dyn_state.course = physpos.orien.get_course();
    _state.pitch = physpos.orien.get_pitch();
    _state.roll  = physpos.orien.get_roll();

	//if (force || tow_attached_ || (cg::distance2d(fmspos.pos, physpos.pos) > cg::norm(physpos.dpos) * prediction * sys_->calc_step() * 2))
	//	aircraft_fms::model_control_ptr(get_fms_info())->reset_pos(physpos.pos, physpos.orien.cpr().course);
#endif

}

void model::on_state(msg::state_msg const& msg)
{
    FIXME(useless or useful)
    //fms::pilot_state_t fmsst = (state_t)msg;

    //if (pilot_impl_)
    //    fms::pilot_simulation_ptr(pilot_impl_)->reset_state(fmsst);

    view::on_state(msg);
}

void model::on_child_removing(kernel::object_info_ptr child)
{
    bool had_nodes_manager = get_nodes_manager();

    view::on_child_removing(child);

    if (had_nodes_manager && !get_nodes_manager())
    {
        root_.reset();
        elev_rudder_node_.reset();
        rudder_node_.reset();
        tow_point_node_.reset();
        body_node_.reset();
    }
}

void model::on_object_destroying(object_info_ptr object)
{
    view::on_object_destroying(object);

    if (tow_attached_ && object->object_id() == *tow_attached_)
    {
        set_tow_attached(boost::none, boost::function<void()>());
    }
}


point_3 model::tow_offset() const
{
    return tow_point_node_ ? get_nodes_manager()->get_relative_transform(/*get_nodes_manager(),*/ tow_point_node_, body_node_).translation() : point_3();
}

bool model::tow_attached() const
{
    return tow_attached_;
}

geo_position model::get_phys_pos() const
{
    // TODO
    return geo_position();//phys_aircraft_->get_position();
}

double model::rotors_angular_speed() const
{
    return rotors_angular_speed_ ;
}

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

    //if (phys_aircraft_)
    //    phys_aircraft_->set_steer(steer);
}

void model::set_brake( double brake )
{   
    Assert(tow_attached_);

    //if (phys_aircraft_)
    //    phys_aircraft_->set_brake(brake);
}

 
geo_position model::fms_pos() const
{
    point_3 dir = cg::polar_point_3(1., /*get_fms_info()->*/get_state().orien().course, /*get_fms_info()->*/get_state().orien().pitch);
    return geo_position(/*get_fms_info()->*/get_state().dyn_state.pos, /*get_fms_info()->*//*get_state().dyn_state.TAS * dir*/point_3(0,0,0), /*get_fms_info()->*/get_state().orien(), point_3(0,0,0));
}

void model::switch_sync_state(sync_fsm::state_ptr state)
{
    if (sync_state_)
        sync_state_->deinit();
    sync_state_ = state;
}

void model::freeze_position()
{
    auto  fmspos = fms_pos();
    nodes_management::node_position root_node_pos(geo_position(fmspos.pos, fmspos.orien));
    root_->set_position(root_node_pos);
}

void model::set_phys_aircraft(phys_aircraft_ptr phys_aircraft)
{
    /*   if (!phys_aircraft)
    {
    if (tow_attached_ && tow_invalid_callback_)
    tow_invalid_callback_();
    }
    phys_aircraft_ = phys_aircraft;*/
}

void model::set_nm_angular_smooth(double val)
{
    nm_ang_smooth_ = val;
}

void model::set_rotors_angular_speed(double val)
{
    rotors_angular_speed_ = val;
}


void model::check_wheel_brake()
{
    //if (!phys_aircraft_)
    //    return;

    //shassis_->visit_groups([this](shassis_group_t & shassis_group)
    //{
    //    if (shassis_group.opened && shassis_group.malfunction && !shassis_group.broken)
    //    {
    //        bool has_contact = shassis_group.check_contact(this->phys_aircraft_);
    //        if (has_contact)
    //            shassis_group.broke(this->phys_aircraft_);
    //    }
    //});
}

void model::check_rotors_malfunction()
{
    //if (!phys_aircraft_)
    //    return;

    rotors_->visit_groups([this](aircraft::rotors_group_t & rotors_group,size_t& ind)
    {
        if ( rotors_group.malfunction )
        {
            rotors_group.angular_velocity(0);
        }
        else
            rotors_group.angular_velocity(rotors_angular_speed_);
    });
}



void model::on_time_factor_changed(double /*time*/, double factor)
{
    bool fast = factor > 1 ? true : false;

    if (fast_session_ != fast)
    {

        if (sync_state_)
        {
            sync_fsm::state_ptr prev_state = sync_state_;
            sync_state_->on_fast_session(fast);
        }
        fast_session_ = fast;
    }
}


}
