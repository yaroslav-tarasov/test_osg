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
        struct phys_state : state_t
        {
            phys_state(self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
                : self_(self)
                , on_ground_(false)
                , phys_aircraft_(phys_aircraft)
                , base_(base)
            {
                self.set_nm_angular_smooth(2);
                self_.set_phys_aircraft(phys_aircraft_);
            }

            ~phys_state()
            {
            }

            void deinit()
            {
                if (self_.get_shassis())
                    self_.get_shassis()->freeze();
                self_.set_phys_aircraft(nullptr);
            }

            void update(double /*time*/, double dt);
            void on_zone_destroyed( size_t id );
            void on_fast_session( bool fast );


        protected:
            void sync_wheels(double dt);
            void sync_rotors(double dt);

        protected:
            self_t &self_;
            geo_base_3 base_;
            size_t zone_;
            phys_aircraft_ptr phys_aircraft_;
            bool on_ground_;

        //public:
        //    static const double phys_height;
        };

        struct phys_state2 : phys_state
        {
            phys_state2(self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
                    : phys_state(self,phys_aircraft,base)
                    , desired_velocity_(aircraft::min_desired_velocity())
                    { }

            void update(double /*time*/, double dt) override;

            double                                 desired_velocity_;
        };

        sync_fsm::state_ptr create_sync_phys_state(phys_state_t type,self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
        {
            if(type!=TEST_NEW)
                return boost::make_shared<phys_state>(self,phys_aircraft,base);
            else
                return boost::make_shared<phys_state2>(self,phys_aircraft,base);
        }

    }
}


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
        sync_rotors(dt);

        auto fmspos = self_.fms_pos();

        if (fmspos.pos.height > phys_height())
        {
            self_.switch_sync_state(boost::make_shared<transition_phys_fms_state>(self_, phys_aircraft_, time));
        }
    }

    void phys_state2::update(double time, double dt) 
    {
        if (!phys_aircraft_)
            return;


        if(auto traj_ = self_.get_trajectory())
        {
            auto it = this;

            if (traj_->cur_len() < traj_->length())
            {
                phys_aircraft_->set_prediction(15.); 
                phys_aircraft_->freeze(false);
                const double  cur_len = traj_->cur_len();
                traj_->set_cur_len (traj_->cur_len() + dt*desired_velocity_);
                const double  tar_len = traj_->cur_len();
                decart_position target_pos;

                target_pos.pos = cg::point_3(traj_->kp_value(tar_len),0);
                geo_position gtp(target_pos, get_base());
                phys_aircraft_->go_to_pos(gtp.pos ,gtp.orien);

                auto physpos = phys_aircraft_->get_position();

                self_.set_desired_nm_pos(physpos.pos);
                self_.set_desired_nm_orien(physpos.orien);

                const double curs_change = traj_->curs_value(tar_len) - traj_->curs_value(cur_len);

                if(cg::eq(curs_change,0.0))
                    desired_velocity_ = aircraft::max_desired_velocity();
                else
                    desired_velocity_ = aircraft::min_desired_velocity();

                const decart_position cur_pos = phys_aircraft_->get_local_position();

                logger::need_to_log(true);

                LOG_ODS_MSG(
                    "curr_pods_len:  "                << traj_->cur_len() 
                    << "    desired_velocity :  "     << desired_velocity_   
                    << "    delta curs :  "           << curs_change
                    << ";   cur_pos x= "              << cur_pos.pos.x << " y= "  << cur_pos.pos.y  
                    << "    target_pos x= "           << target_pos.pos.x << " y= "  << target_pos.pos.y << "\n" 
                    );

                logger::need_to_log(false);

            }
            else
            {

                cg::point_3 cur_pos = phys_aircraft_->get_local_position().pos;
                cg::point_3 d_pos   = phys_aircraft_->get_local_position().dpos;
                cg::point_3 trg_p(traj_->kp_value(traj_->length()),0);
                d_pos.z = 0;
                if(cg::distance(trg_p,cur_pos) > 1.0 && cg::norm(d_pos) > 0.05)
                {   
                    decart_position target_pos;
                    target_pos.pos = trg_p;
                    geo_position gp(target_pos, get_base());
                    phys_aircraft_->go_to_pos(gp.pos ,gp.orien);
                }
                else
                {
                    // traj.reset();
                    phys_aircraft_->freeze(true);
                }

                auto physpos = phys_aircraft_->get_position();

                self_.set_desired_nm_pos(physpos.pos);
                self_.set_desired_nm_orien(physpos.orien);

            }

            phys_aircraft_->update();
        }
        else
        {
            fms::procedure_model_ptr pm =  self_.get_fms_info()->procedure_model();
            double prediction = cg::clamp(pm->taxi_TAS(), pm->takeoff_TAS(), 15., 30.)(self_.get_fms_info()->get_state().dyn_state.TAS);
            //geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*0.1));

            //LOG_ODS_MSG( "TAS:  "  << self_.get_fms_info()->get_state().dyn_state.TAS << "\n" );

            geo_position physpos = phys_aircraft_->get_position();
            //physpos.pos += point_3(20.1,20.1,0);
            geo_base_3 predict_pos  = physpos.pos;

            // phys_aircraft_->go_to_pos(predict_pos, self_.get_fms_info()->get_state().orien());
            phys_aircraft_->go_to_pos(predict_pos, physpos.orien);
            phys_aircraft_->set_air_cfg(self_.get_fms_info()->get_state().dyn_state.cfg);
            phys_aircraft_->set_prediction(prediction);

            phys_aircraft_->update();

            physpos = phys_aircraft_->get_position();
            self_.set_desired_nm_pos(physpos.pos);
            self_.set_desired_nm_orien(physpos.orien);
        }


        sync_wheels(dt);
        sync_rotors(dt);

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
#ifdef OSG_NODE_IMPL
            nodes_management::node_info_ptr rel_node = wnode;
#else
            nodes_management::node_info_ptr rel_node = wnode->rel_node();
#endif            

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

    FIXME(А оно нам надо? Здесь?)
    void phys_state::sync_rotors(double dt)
    {
        self_.get_rotors()->visit_rotors([this, dt](rotors_group_t const& rg,size_t& id)
        {
            auto wnode = rg.node;

            geo_position wpos;

            const float ob_min = 45;
            nodes_management::node_position wheel_node_pos = wnode->position();
            const float angular_velocity = ob_min * 2 * osg::PI/60.0; // 2000 и 3000 об/мин (30-50 об/с) 

            quaternion des_orien;
            des_orien = wheel_node_pos.local().orien * quaternion(cpr(0,0,cg::rad2grad() * angular_velocity * dt));

            const cg::transform_4 wheel_node_trans = cg::transform_4(cg::as_translation(-wheel_node_pos.local().pos), /*wpos.orien*/des_orien.rotation()); 
            point_3 omega_rel     = cg::get_rotate_quaternion(wheel_node_pos.local().orien,des_orien).rot_axis().omega() / (dt);

            // wheel_node_pos.local().orien = /*wpos.orien*/wheel_node_trans.rotation().quaternion();
            wheel_node_pos.local().omega = omega_rel;

            wnode->set_position(wheel_node_pos);

        });
    }

}
}
