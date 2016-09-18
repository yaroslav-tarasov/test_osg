#include "stdafx.h"
#include "precompiled_objects.h"

#include "sync_fms_state.h"
#include "sync_none_state.h"
#include "sync_phys_state.h"
#include "sync_transition_fms_phys.h"

namespace aircraft
{

namespace sync_fsm
{
	sync_fsm::state_ptr create_fms_state(phys_state_t type,self_t &self);

	sync_fsm::state_ptr create_fms_state(self_t &self)
	{
		return create_fms_state(EXTERNAL, self);
	};

	struct fms_state : state_t
	{
		fms_state(self_t &self)
			: self_(self)
			, phys_aircraft_failed_(false)
		{
			auto fms_pos = self_.fms_pos();

			if (fms_pos.pos.height > 1)
				self.set_nm_angular_smooth(25);
			else
				self.set_nm_angular_smooth(5);

		}

		void update(double /*time*/, double dt) ;


		self_t &self_;
		bool phys_aircraft_failed_;
	};
	
	// from phl
	struct fms_state3 : fms_state
	{
		fms_state3(self_t &self)
			: fms_state(self)
		    , desired_velocity_(aircraft::min_desired_velocity())
		{
			auto fms_pos = self_.fms_pos();

			if (fms_pos.pos.height > 1)
				self.set_nm_angular_smooth(25);
			else
				self.set_nm_angular_smooth(5);

		}

		void update(double /*time*/, double dt) override;

		double                                 desired_velocity_;
		boost::optional<double>                traj_time_offset_;
	};

	sync_fsm::state_ptr create_fms_state(phys_state_t type,self_t &self)
	{
		if( type != EXTERNAL )
			return boost::make_shared<fms_state>(self);
		else
			return boost::make_shared<fms_state3>(self);
	}


    void fms_state::update(double time, double /*dt*/) 
    {
        //             if (phys_transition_start_ && time > *phys_transition_start_ + phys_transition_time_)
        //             {
        //                 freeze_chassis_group();
        //                 phys_aircraft_.reset();
        //                 phys_transition_start_.reset();
        //                 visit_chassis([](shassis_group_t const&, shassis_t & shassis)
        //                 {
        //                     shassis.clear_wheels();
        //                 });
        //             }
        //             sync_nodes_manager_with_fms(time);

        auto fms_pos = self_.fms_pos();

        self_.set_desired_nm_pos(fms_pos.pos);
        self_.set_desired_nm_orien(fms_pos.orien);

        if (fms_pos.pos.height > 1)
            self_.set_nm_angular_smooth(25);
        else
            self_.set_nm_angular_smooth(5);

        airport::info_ptr airprt = self_.get_airports_manager()->find_closest_airport(fms_pos.pos);


        // transitions
        if (!self_.is_fast_session() && !phys_aircraft_failed_ && fms_pos.pos.height < /*phys_state::*/phys_height() - 1 && self_.phys_control()->get_zone(fms_pos.pos))
        {
            optional<size_t> phys_zone = self_.phys_control()->get_zone(fms_pos.pos);
            if (phys_zone)
            {
                auto base = self_.phys_control()->get_base(*phys_zone);
                auto phys_sys = self_.phys_control()->get_system(*phys_zone);
                                                                                              // FIXME we need it
                phys_aircraft_ptr phys_aircraft = phys_aircraft_impl::create(base, phys_sys, /*self_.get_meteo_cursor(),*/ 
                    self_.get_nodes_manager(), self_.fms_pos(), 
                    *self_.get_fms_info()->fsettings(), self_.get_shassis(), *phys_zone);

                if (phys_aircraft)
                {
                    if (!cg::eq_zero(fms_pos.pos.height))
                        self_.switch_sync_state(create_transition_fms_phys_state(self_, phys_aircraft, base, time));
                    else
                        self_.switch_sync_state(create_sync_phys_state(self_, phys_aircraft, base));

                }
            }
        }
        else if (cg::distance2d(airprt->pos(), fms_pos.pos) > none_state::sync_none_dist)
        {
            self_.switch_sync_state(boost::make_shared<none_state>(self_));
        }

    }



	void fms_state3::update(double time, double dt) 
	{
 		auto fms_pos = self_.fms_pos();

		if(auto traj_ = self_.get_trajectory())
		{

			traj_->set_cur_len ((time>0)? time:0.0);
			const double  tar_len = traj_->cur_len();
			decart_position target_pos;

			if(traj_->speed_value(tar_len))
			{
				const double speed = *traj_->speed_value(tar_len);
				//force_log fl;       
				LOG_ODS_MSG( "phys_state2::update " << tar_len << "  speed= " << speed << "\n"
					);

				//self_.set_desired_nm_speed(speed);
			}

			target_pos.pos = cg::point_3(traj_->kp_value(tar_len));
			target_pos.orien = traj_->curs_value(tar_len);

			target_pos.dpos = (target_pos.pos - cg::point_3(traj_->kp_value(tar_len - dt))) / (/*sys_->calc_step()*/dt);
			geo_position gtp(target_pos, get_base());


			self_.set_desired_nm_pos(gtp.pos);
			self_.set_desired_nm_orien(gtp.orien);
			
			fms_pos = gtp;
  	   }



#if 0
		self_.set_desired_nm_pos(fms_pos.pos);
		self_.set_desired_nm_orien(fms_pos.orien);
#endif

		if (fms_pos.pos.height > 1)
			self_.set_nm_angular_smooth(25);
		else
			self_.set_nm_angular_smooth(5);

		airport::info_ptr airprt = self_.get_airports_manager()->find_closest_airport(fms_pos.pos);


		// transitions
		if (!self_.is_fast_session() && !phys_aircraft_failed_ && fms_pos.pos.height < /*phys_state::*/phys_height() - 1 && self_.phys_control()->get_zone(fms_pos.pos))
		{
			optional<size_t> phys_zone = self_.phys_control()->get_zone(fms_pos.pos);
			if (phys_zone)
			{
				auto base = self_.phys_control()->get_base(*phys_zone);
				auto phys_sys = self_.phys_control()->get_system(*phys_zone);
				// FIXME we need it
				phys_aircraft_ptr phys_aircraft = phys_aircraft_impl::create(base, phys_sys, /*self_.get_meteo_cursor(),*/ 
					self_.get_nodes_manager(), fms_pos, 
					*self_.get_fms_info()->fsettings(), self_.get_shassis(), *phys_zone);

				if (phys_aircraft)
				{
					if (!cg::eq_zero(fms_pos.pos.height))
						self_.switch_sync_state(create_transition_fms_phys_state(self_, phys_aircraft, base, time));
					else
						self_.switch_sync_state(create_sync_phys_state(self_, phys_aircraft, base));

				}
			}
		}
		else if (cg::distance2d(airprt->pos(), fms_pos.pos) > none_state::sync_none_dist)
		{
			self_.switch_sync_state(boost::make_shared<none_state>(self_));
		}

	}


}
}
