#include "stdafx.h"
#include "precompiled_objects.h"

#include "sync_phys_state.h"
#include "sync_transition_phys_fms.h"
#include "sync_none_state.h"
#include "sync_fms_state.h"

namespace aircraft
{
namespace sync_fsm
{
    void phys_state::update(double time, double dt) 
    {
        if (!phys_aircraft_)
            return;

        fms::procedure_model_ptr pm =  self_.get_fms_info()->procedure_model();
        double prediction = cg::clamp(pm->taxi_TAS(), pm->takeoff_TAS(), 15., 30.)(self_.get_fms_info()->get_state().dyn_state.TAS);

        geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*0.1));
        phys_aircraft_->go_to_pos(predict_pos, self_.get_fms_info()->get_state().orien());
        phys_aircraft_->set_air_cfg(self_.get_fms_info()->get_state().dyn_state.cfg);
        phys_aircraft_->set_prediction(prediction);

        phys_aircraft_->update();

        //             if (fms_info_->get_state().cfg == fms::CFG_GD)
        //             {
        //                 bool on_ground = phys_aircraft_->has_contact();
        // 
        //                 if (on_ground || !on_ground_time_ || time > *on_ground_time_ + 5)
        //                 {
        //                     on_ground_ = on_ground;
        //                     on_ground_time_ = time;
        //                 }
        //             }
        //             else
        //                 on_ground_ = false;
        // 

        auto physpos = phys_aircraft_->get_position();

        self_.set_desired_nm_pos(physpos.pos);
        self_.set_desired_nm_orien(physpos.orien);

        sync_wheels(dt);

        auto fmspos = self_.fms_pos();

        if (fmspos.pos.height > phys_height)
        {
            self_.switch_sync_state(boost::make_shared<transition_phys_fms_state>(self_, phys_aircraft_, time));
        }
    }

    void phys_state::on_zone_destroyed( size_t id )
    {
        if (phys_aircraft_->get_zone() == id)
            self_.switch_sync_state(boost::make_shared<none_state>(self_));
    }

    void phys_state::on_fast_session( bool fast )
    {
        if (fast)
            self_.switch_sync_state(boost::make_shared<fms_state>(self_));
    }


    void phys_state::sync_wheels(double dt)
    {
        geo_position root_pos = self_.get_root_pos();

        quaternion root_next_orien = quaternion(cg::rot_axis(root_pos.omega * dt)) * root_pos.orien;
        geo_base_3 root_next_pos = root_pos.pos(root_pos.dpos * dt);

        geo_position body_pos = phys_aircraft_->get_position();

        self_.get_shassis()->visit_chassis([this, &root_next_orien, &root_next_pos, &body_pos, dt](shassis_group_t const&, shassis_t & shassis)
        {
            auto wnode = shassis.wheel_node;
            auto chassis_node = shassis.node;

            if (shassis.phys_wheels.empty())
                return;

            geo_position wpos = this->phys_aircraft_->get_wheel_position(shassis.phys_wheels[0]);

            quaternion wpos_rel_orien = (!body_pos.orien) * wpos.orien;
            point_3 wpos_rel_pos = (!body_pos.orien).rotate_vector(body_pos.pos(wpos.pos));
// FIXME TODO
            //nodes_management::node_info_ptr rel_node = wnode->rel_node();
            nodes_management::node_info_ptr rel_node = wnode;

            geo_base_3 global_pos = wnode->get_global_pos();
            quaternion global_orien = wnode->get_global_orien();

            transform_4 rel_node_root_tr = rel_node->get_root_transform();

            point_3    desired_pos_in_rel = rel_node_root_tr.inverted() * wpos_rel_pos;
            quaternion desired_orien_in_rel = (!quaternion(rel_node_root_tr.rotation().cpr())) * wpos_rel_orien;

            desired_orien_in_rel = quaternion(cpr(0, 0, -root_next_orien.get_roll())) * desired_orien_in_rel;

            nodes_management::node_position wheel_node_pos = wnode->position();
            nodes_management::node_position chassis_node_pos = chassis_node->position();

            chassis_node_pos.local().dpos.z = (desired_pos_in_rel.z - wheel_node_pos.local().pos.z) / dt;

            quaternion q = cg::get_rotate_quaternion(wheel_node_pos.local().orien, desired_orien_in_rel);
            point_3 omega_rel     = cg::get_rotate_quaternion(wheel_node_pos.local().orien, desired_orien_in_rel).rot_axis().omega() / (dt);
            wheel_node_pos.local().omega = omega_rel;

            wnode->set_position(wheel_node_pos);
            chassis_node->set_position(chassis_node_pos);
        });
    }

}
}
