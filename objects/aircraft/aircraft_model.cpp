#include "stdafx.h"

#include "aircraft_model.h"

namespace aircraft
{

// from view
#pragma region view
cg::transform_4 const&  model::tow_point_transform() const
{
    return tow_point_transform_;
}

nodes_management::node_info_ptr model::root() const
{
    //return nodes_manager_->get_node(0);  // FIXME отступаем от исходной модели
    return nodes_manager_->find_node("root");
}

bool model::malfunction(malfunction_kind_t kind) const
{
    return malfunctions_[kind];
}

cg::geo_point_3 const& model::pos() const
{
    static cg::geo_point_3 pp;
    return pp;//fms_info_->get_state().dyn_state.pos;
}

cg::point_3 model::dpos() const
{
    return cg::point_3();// cg::polar_point_3(fms_info_->get_state().dyn_state.TAS, fms_info_->get_state().orien().course, fms_info_->get_state().orien().pitch);
}

cg::cpr model::orien() const
{
    return cg::cpr();//fms_info_->get_state().orien();
}

#pragma  endregion

void model::on_malfunction_changed( malfunction_kind_t kind ) 
{
    if (kind == MF_CHASSIS_FRONT)
    {
        shassis_->set_malfunction(SG_FRONT, malfunction(kind));
    }
    else if (kind == MF_CHASSIS_REAR_LEFT)
    {
        shassis_->set_malfunction(SG_REAR_LEFT, malfunction(kind));
    }
    else if (kind == MF_CHASSIS_REAR_RIGHT)
    {
        shassis_->set_malfunction(SG_REAR_RIGHT, malfunction(kind));
    }
    else if (kind == MF_ONE_ENGINE || kind == MF_ALL_ENGINES)
    {
        double factor = 1;
        if (malfunction(MF_ALL_ENGINES))
            factor = 0;
        else if (malfunction(MF_ONE_ENGINE))
            factor = 0.7;

        // FIXME TODO OR NOT TODO
        //auto controls = flight_manager_control_->get_controls();
        //controls.engine_factor = factor;
        //flight_manager_control_->set_controls(controls);
    }
    else if (kind == MF_FUEL_LEAKING)
    {   
        // FIXME TODO OR NOT TODO
        //auto controls = flight_manager_control_->get_controls();
        //controls.fuel_leaking = true;
        //flight_manager_control_->set_controls(controls);
    }
}

void model::update_contact_effects(double time)
{
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
                point_3 loc_omega = (!body_pos.orien).rotate_vector(wpos.omega);
                point_3 vel = body_pos.dpos - body_pos.orien.rotate_vector(point_3(loc_omega.x,0,0) * cg::grad2rad()) * shassis.radius;

                if (cg::norm(vel) > 50)
                {
                    point_3 loc_offset = (!body_pos.orien).rotate_vector(body_pos.pos(wpos.pos));
                    loc_offset += point_3(0,0,shassis.radius);
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
}

void model::update(double dt)
{
    auto it = this;

    if((*it).traj_.get())
    {
        if ((*it).traj_->cur_len() < (*it).traj_->length())
        {
            (*it).phys_aircraft_->set_prediction(15.); 
            (*it).phys_aircraft_->freeze(false);
            const double  cur_len = (*it).traj_->cur_len();
            (*it).traj_->set_cur_len ((*it).traj_->cur_len() + dt*(*it).desired_velocity_);
            const double  tar_len = (*it).traj_->cur_len();
            decart_position target_pos;

            target_pos.pos = cg::point_3((*it).traj_->kp_value(tar_len),0);
            geo_position gtp(target_pos, get_base());
            (*it).phys_aircraft_->go_to_pos(gtp.pos ,gtp.orien);


            const double curs_change = (*it).traj_->curs_value(tar_len) - (*it).traj_->curs_value(cur_len);

            if(cg::eq(curs_change,0.0))
                (*it).desired_velocity_ = aircraft::model::max_desired_velocity;
            else
                (*it).desired_velocity_ = aircraft::model::min_desired_velocity;

            // const decart_position cur_pos = _phys_aircrafts[0].phys_aircraft_->get_local_position();

            //std::stringstream cstr;

            //cstr << std::setprecision(8) 
            //     << "curr_pods_len:  "             << (*it).traj->cur_len() 
            //     << "    desired_velocity :  "     << (*it).desired_velocity_   
            //     << "    delta curs :  "  << curs_change
            //     << ";   cur_pos x= "     << cur_pos.pos.x << " y= "  << cur_pos.pos.y  
            //     << "    target_pos x= "  << target_pos.pos.x << " y= "  << target_pos.pos.y <<"\n" ;

            //OutputDebugString(cstr.str().c_str());
        }
        else
        {

            cg::point_3 cur_pos = phys_aircraft_->get_local_position().pos;
            cg::point_3 d_pos = phys_aircraft_->get_local_position().dpos;
            cg::point_3 trg_p((*it).traj_->kp_value((*it).traj_->length()),0);
            d_pos.z = 0;
            if(cg::distance(trg_p,cur_pos) > 1.0 && cg::norm(d_pos) > 0.05)
            {   
                decart_position target_pos;
                target_pos.pos = trg_p;
                geo_position gp(target_pos, get_base());
                (*it).phys_aircraft_->go_to_pos(gp.pos ,gp.orien);
            }
            else
            {
                // (*it).traj.reset();
                (*it).phys_aircraft_->freeze(true);
            }
        }

    }

    phys_aircraft_->update();

}


}
