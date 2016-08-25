#include "stdafx.h"
#include "precompiled_objects.h"


//#include "objects/helicopter_physless.h"
//#include "helicopter_physless/helicopter_physless_common.h"

#include "sync_heli_phys_state.h"
#include "sync_heli_transition_phys_fms.h"
#include "sync_heli_none_state.h"
#include "sync_heli_fms_state.h"

namespace helicopter_physless
{
    namespace sync_fsm
    {

        struct phys_state2 : state_t
        {
            phys_state2(self_t &self, aircraft::phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
                : self_(self)
                , desired_speed_(aircraft::min_desired_velocity())
                , on_ground_(false)
                , phys_aircraft_(phys_aircraft)
                , base_(base)
            {
                self.set_nm_angular_smooth(2);
                self_.set_phys_model(phys_aircraft_);
            }

            ~phys_state2()
            {
            }

            void deinit()
            {
                if (self_.get_shassis())
                    self_.get_shassis()->freeze();
                self_.set_phys_model(nullptr);
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


            double                                 desired_speed_;
        };


        sync_fsm::state_ptr create_sync_phys_state(phys_state_t type,self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
        {
                return boost::make_shared<phys_state2>(self,phys_aircraft,base);
        }

    }
}


namespace helicopter_physless
{

const double packet_delay = 1.0;

namespace sync_fsm
{
    const double comfortable_acceleration  = 2.5;
    const double max_dx =   aircraft::max_desired_velocity() * aircraft::max_desired_velocity() / comfortable_acceleration *.5; 

    void phys_state2::update(double time, double dt) 
    {
        if (!phys_aircraft_)
            return;

#if 0   // local state

        if(auto traj_ = self_.get_trajectory())
        {
             const double  cur_len = traj_->cur_len();

            if (cur_len < traj_->length())
            {
                phys_aircraft_->set_prediction(/*15.*/30.); 
                phys_aircraft_->freeze(false);
               
                traj_->set_cur_len (traj_->cur_len() + dt*desired_velocity_);
                const double  tar_len = traj_->cur_len();
                decart_position target_pos;

                target_pos.pos = cg::point_3(traj_->kp_value(tar_len));
                target_pos.orien = traj_->curs_value(tar_len);//cg::cpr(traj_->curs_value(tar_len),0,0);
                geo_position gtp(target_pos, get_base());
                phys_aircraft_->go_to_pos(gtp.pos ,gtp.orien);

                if(gtp.pos.height > 0)
                {
                    phys_aircraft_->set_air_cfg(fms::CFG_TO/*self_.get_fms_info()->get_state().dyn_state.cfg*/);
                }

                FIXME(Нужен позишн я я)
                auto physpos = gtp; //phys_aircraft_->get_position();

                self_.set_desired_nm_pos(physpos.pos);
                self_.set_desired_nm_orien(physpos.orien);

                ///const double curs_change = traj_->curs_value(tar_len) - traj_->curs_value(cur_len);
                
                double current_velocity = desired_velocity_;

                if(traj_->speed_value(tar_len))
                {
                    desired_velocity_ = *traj_->speed_value(tar_len);
                }
                else
                {
                    if(cg::eq(traj_->curs_value(tar_len).cpr(),traj_->curs_value(cur_len).cpr(),0.085))
                        desired_velocity_ = aircraft::max_desired_velocity();
                    else
                        desired_velocity_ = aircraft::min_desired_velocity();
                }

                if (traj_->length() - cur_len < max_dx  )
                {
                    desired_velocity_ = sqrt((traj_->length() - cur_len) / comfortable_acceleration * .5)  ;
                }

                if (current_velocity > desired_velocity_)
                    desired_velocity_ =  current_velocity - comfortable_acceleration * dt;
                else if (current_velocity < desired_velocity_)
                    desired_velocity_ =  current_velocity + comfortable_acceleration * dt;

 

                const decart_position cur_pos = phys_aircraft_->get_local_position();

                {

                    //force_log fl;

                    LOG_ODS_MSG(
                        "curr_pods_len:  "                << traj_->cur_len() 
                        << "    desired_velocity :  "     << desired_velocity_   
                        << "    delta curs :  "           << traj_->curs_value(tar_len).get_course() - traj_->curs_value(cur_len).get_course()
                        //<< ";   cur_pos x= "              << cur_pos.pos.x << " y= "  << cur_pos.pos.y  
                        << "    target_pos x= "           << target_pos.pos.x << " y= "  << target_pos.pos.y << "\n" 
                        );

                }

            }
            else
            {
#if 0
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
#endif
                auto physpos = self_.fms_pos();//phys_aircraft_->get_position();

                //self_.set_desired_nm_pos(physpos.pos);
                //self_.set_desired_nm_orien(physpos.orien);
                {
                    //force_log fl;

                    LOG_ODS_MSG (
                        " physpos.lat " << physpos.pos.lat <<
                        " physpos.lon " << physpos.pos.lon <<
                        " \n"
                    );
                }
                
                self_.freeze_position();

            }

            // phys_aircraft_->update();
        }
        else
        {
#if 0
            fms::procedure_model_ptr pm =  self_.get_fms_info()->procedure_model();
            double prediction = cg::clamp(pm->taxi_TAS(), pm->takeoff_TAS(), 15., 30.)(self_.get_fms_info()->get_state().dyn_state.TAS);
            //geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*0.1));
            self_.get_fms_info()->get_mass();
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
#endif
            auto physpos = self_.fms_pos(); //phys_aircraft_->get_position();

            self_.set_desired_nm_pos(physpos.pos);
            self_.set_desired_nm_orien(physpos.orien);
            
            self_.freeze_position();
        }
#else  // extern state
        FIXME(extern state);
        if(auto traj_ = self_.get_trajectory())
        {

            traj_->set_cur_len ((time-packet_delay>0)? time - packet_delay:0.0/*traj_->cur_len() + dt*/);
            const double  tar_len = traj_->cur_len();
            decart_position target_pos;

#if 1
            if(tar_len > traj_->length())
            {
                force_log fl;       
                LOG_ODS_MSG( "phys_state2::update tar_len > traj_->length()" << tar_len << "  traj_->length() - traj_->base_length()= " << traj_->length() - traj_->base_length() << "\n"
                << "  traj_->length() " << traj_->length() << " traj_->base_length()= " << traj_->base_length()    << "\n" 
                );           
            }
#endif
            geo_position pha_pos = phys_aircraft_->get_position();

            target_pos.pos = cg::point_3(traj_->kp_value(tar_len));
            target_pos.orien = traj_->curs_value(tar_len);

            FIXME(dirty tangage trick)
            target_pos.orien = cpr(target_pos.orien.get_course(),-target_pos.orien.get_pitch(),target_pos.orien.get_roll());
            geo_position gtp(target_pos, get_base());

            const double step = cg::distance2d(pha_pos.pos, gtp.pos);

            self_.set_desired_nm_pos(gtp.pos);
            self_.set_desired_nm_orien(gtp.orien);

#if 0
            force_log fl;       
            LOG_ODS_MSG( "phys_state2::update " << step  << "\n" 
                );  
#endif 

            phys_aircraft_->go_to_pos(gtp.pos, gtp.orien);
			phys_aircraft_->update();
        }
#endif

        sync_wheels(dt);
        sync_rotors(dt);

    }

    void phys_state2::on_zone_destroyed( size_t id )
    {
        if (phys_aircraft_->get_zone() == id)
            self_.switch_sync_state(boost::make_shared<none_state>(self_));
    }

    void phys_state2::on_fast_session( bool fast )
    {
        if (fast)
            self_.switch_sync_state(boost::make_shared<fms_state>(self_));
    }


    void phys_state2::sync_wheels(double dt)
    {
        geo_position root_pos = self_.get_root_pos();

        quaternion root_next_orien = quaternion(cg::rot_axis(root_pos.omega * dt)) * root_pos.orien;
        geo_base_3 root_next_pos = root_pos.pos(root_pos.dpos * dt);
        
        logger::need_to_log(true);
        
		FIXME(Model position);

        geo_position body_pos = root_pos; //phys_aircraft_->get_position();

        self_.get_shassis()->visit_chassis([this, &root_next_orien, &root_next_pos, &body_pos, dt](aircraft::shassis_group_t const& gr, aircraft::shassis_t & shassis)
        {

            auto wnode = shassis.wheel_node;
            auto chassis_node = shassis.node;

            //if (shassis.phys_wheels.empty())
            //    return;
			
			geo_base_3 global_pos   = wnode->get_global_pos();
			quaternion global_orien = wnode->get_global_orien();

            geo_position wpos = this->phys_aircraft_->get_wheel_position(shassis.phys_wheels[0]);
            
            // quaternion wpos_rel_orien_orig =  (!body_pos.orien) * wpos.orien;

            quaternion wpos_rel_orien =  (!body_pos.orien) * global_orien;
            //quaternion(cpr(wpos_rel_orien_orig.get_course(), global_orien.get_pitch(),0))/*wpos.orien*/;
            point_3 wpos_rel_pos = (!body_pos.orien).rotate_vector(body_pos.pos(/*wpos.pos*/global_pos));
           
#ifdef OSG_NODE_IMPL
            nodes_management::node_info_ptr rel_node = wnode;
#else
            nodes_management::node_info_ptr rel_node = wnode->rel_node();
#endif            

            transform_4 rel_node_root_tr = rel_node->get_root_transform();

            point_3    desired_pos_in_rel = rel_node_root_tr.inverted() * wpos_rel_pos;
            quaternion desired_orien_in_rel = (!quaternion(rel_node_root_tr.rotation().cpr())) * wpos_rel_orien;

            desired_orien_in_rel = quaternion(cpr(0, 0, -root_next_orien.get_roll())) * desired_orien_in_rel;

            // -----------------------------------------------------------------------

            nodes_management::node_position wheel_node_pos = wnode->position();
            nodes_management::node_position chassis_node_pos = chassis_node->position();

            chassis_node_pos.local().dpos.z = (desired_pos_in_rel.z - wheel_node_pos.local().pos.z) / dt;
			
			double wr = get_wheel_radius(wnode);
			cg::rectangle_3 wbb = wnode->get_bound();
#if 0
            const float angular_speed = 45 * 2 * cg::pif/60.0; 
#endif
            point_3 forward_dir = cg::normalized_safe(body_pos.orien.rotate_vector(point_3(0, 1, 0))) ;

            point_3 wpos_rp = (!body_pos.orien).rotate_vector(body_pos.dpos);
            auto delta = cg::sign(forward_dir * body_pos.dpos) * sqrt(wpos_rp.x * wpos_rp.x  + wpos_rp.y * wpos_rp.y);
			// auto delta = sqrt(body_pos.dpos.x * body_pos.dpos.x  + body_pos.dpos.y * body_pos.dpos.y);

			const float angular_speed = delta / wr; 
			desired_orien_in_rel = wheel_node_pos.local().orien * cg::quaternion(cg::cpr(0,-cg::rad2grad() * angular_speed * dt,0));
            //quaternion q = cg::get_rotate_quaternion(wheel_node_pos.local().orien, desired_orien_in_rel);

            cg::point_3 omega_rel       = cg::get_rotate_quaternion(wheel_node_pos.local().orien, desired_orien_in_rel).rot_axis().omega() / dt;
            wheel_node_pos.local().omega = omega_rel ;

            
#if 0
            wnode->set_position(wheel_node_pos);
            chassis_node->set_position(chassis_node_pos);
#endif

        });

        logger::need_to_log(false);
	}

    void phys_state2::sync_rotors(double dt)
    {                                                                                         
        self_.get_rotors()->visit_rotors([this, dt](aircraft::rotors_group_t const& rg,size_t& id)
        {
            auto rnode = rg.node;
            
            geo_position rpos;

            float  ob_min    = rg.ang_speed;
            const double abs_speed = abs(rg.ang_speed);
            ob_min = abs_speed>150?-20*cg::sign(ob_min):ob_min; // Перестаем крутить ротор на высокой скорости 

            nodes_management::node_position rotor_node_pos = rnode->position();
            const float angular_speed = ob_min * 2 * cg::pif/60.0; // 2000 и 3000 об/мин (30-50 об/с) 
           
            quaternion des_orien = rotor_node_pos.local().orien * quaternion(cpr(cg::rad2grad() * angular_speed * dt,0,0));

            // const cg::transform_4 rotor_node_trans = cg::transform_4(cg::as_translation(-rotor_node_pos.local().pos), /*rpos.orien*/des_orien.rotation()); 
            point_3 omega_rel     = cg::get_rotate_quaternion(rotor_node_pos.local().orien,des_orien).rot_axis().omega() / (dt);

            // rotor_node_pos.local().orien = /*rpos.orien*/rotor_node_trans.rotation().quaternion();
            rotor_node_pos.local().omega = omega_rel;

            rnode->set_position(rotor_node_pos);   
            
            if(abs_speed>150.)
            {
                if(rg.dyn_rotor_node)
                {
                    if(!rg.dyn_rotor_node->get_visibility() || rg.dyn_rotor_node->get_visibility() && !*(rg.dyn_rotor_node->get_visibility()))
                    {
                        rg.dyn_rotor_node->set_visibility(true);
                        rg.dyn_rotor_node->set_position(rg.dyn_rotor_node->position());

                        // self_.set_rotors_state(1.0,1.0);
                    }
                }

                if(rg.sag_rotor_node)
                {
                    if(!rg.sag_rotor_node->get_visibility() || rg.sag_rotor_node->get_visibility() && *(rg.sag_rotor_node->get_visibility()))
                    {
                        rg.sag_rotor_node->set_visibility(false);
                        rg.sag_rotor_node->set_position(rg.sag_rotor_node->position());
                    }
                }

                if(rg.rotor_node)
                {
                    if(!rg.rotor_node->get_visibility() || rg.rotor_node->get_visibility() && *(rg.rotor_node->get_visibility()))
                    {
                        rg.rotor_node->set_visibility(false);
                        rg.rotor_node->set_position(rg.rotor_node->position());
                    }
                }

                self_.set_rotors_state(cg::clamp01(1 - abs_speed / 150.), 0.1, rs_dynamic);
            }
            else  if(abs_speed<15.)
            {
                if(rg.dyn_rotor_node)
                {
                    if(!rg.dyn_rotor_node->get_visibility() || rg.dyn_rotor_node->get_visibility() && *(rg.dyn_rotor_node->get_visibility()))
                    {
                        rg.dyn_rotor_node->set_visibility(false);
                        rg.dyn_rotor_node->set_position(rg.dyn_rotor_node->position());
                    }
                }

                if(rg.sag_rotor_node)
                {
                    if(!rg.sag_rotor_node->get_visibility() || rg.sag_rotor_node->get_visibility() && !*(rg.sag_rotor_node->get_visibility()))
                    {
                        rg.sag_rotor_node->set_visibility(true);
                        rg.sag_rotor_node->set_position(rg.sag_rotor_node->position());

                        // self_.set_rotors_state(abs_speed / 150. , 0.1);
                    }
                }

                if(rg.rotor_node)
                {
                    if(!rg.rotor_node->get_visibility() || rg.rotor_node->get_visibility() && *(rg.rotor_node->get_visibility()))
                    {
                        rg.rotor_node->set_visibility(false);
                        rg.rotor_node->set_position(rg.rotor_node->position());
                    }
                }

                self_.set_rotors_state(cg::clamp01(1 - abs_speed / 150.), 0.1, rs_sagged);
            }
            else
            {
                if(rg.dyn_rotor_node)
                {   
                    if(!rg.dyn_rotor_node->get_visibility() || rg.dyn_rotor_node->get_visibility() && *(rg.dyn_rotor_node->get_visibility()))
                    {
                        rg.dyn_rotor_node->set_visibility(false);
                        rg.dyn_rotor_node->set_position(rg.dyn_rotor_node->position());
                    }
                }

                if(rg.sag_rotor_node)
                {
                    if(!rg.sag_rotor_node->get_visibility() || rg.sag_rotor_node->get_visibility() && *(rg.sag_rotor_node->get_visibility()))
                    {
                        rg.sag_rotor_node->set_visibility(false);
                        rg.sag_rotor_node->set_position(rg.sag_rotor_node->position());
#if 0
                        force_log fl;       
                        LOG_ODS_MSG( "phys_state2::sync_rotors(double dt) sag_rotor_node " << "id: " << id << "\n");
#endif

                    }
                }

                if(rg.rotor_node)
                {
                    if(!rg.rotor_node->get_visibility() || rg.rotor_node->get_visibility() && !*(rg.rotor_node->get_visibility()))
                    {
                        rg.rotor_node->set_visibility(true);
                        rg.rotor_node->set_position(rg.rotor_node->position());

                        // self_.set_rotors_state(1.0, 0.1);
                    }
                }

                self_.set_rotors_state(cg::clamp01(1 - abs_speed / 150.), 0.1, rs_static);
            }
        });
    }

}
}
