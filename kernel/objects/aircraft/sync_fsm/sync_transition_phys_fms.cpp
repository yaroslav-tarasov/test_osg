#include "sync_transition_phys_fms.h"
#include "sync_fms_state.h"
#include "sync_none_state.h"

namespace aircraft
{
    namespace sync_fsm
    {

		sync_fsm::state_ptr create_transition_phys_fms_state(phys_state_t type,self_t &self, phys_aircraft_ptr phys_aircraft,  double time);

		sync_fsm::state_ptr create_transition_phys_fms_state(self_t &self, phys_aircraft_ptr phys_aircraft,  double time)
		{
			return create_transition_phys_fms_state(EXTERNAL, self, phys_aircraft, time);
		};


		struct transition_phys_fms_state : state_t
		{
			transition_phys_fms_state(self_t &self, phys_aircraft_ptr phys_aircraft, double time)
				: self_(self)
				, start_transition_time_(time)
				, phys_aircraft_(phys_aircraft)
			{
			}

			void update(double time, double dt);
			void on_zone_destroyed( size_t id );

		private:
		private:
			self_t &self_;
			double start_transition_time_;
			geo_base_3 base_;

			phys_aircraft_ptr phys_aircraft_;
		};


		sync_fsm::state_ptr create_transition_phys_fms_state(phys_state_t type,self_t &self, phys_aircraft_ptr phys_aircraft, double time)
		{
			if( type != EXTERNAL )
				return boost::make_shared<transition_phys_fms_state>(self,phys_aircraft, time);
			else
				return boost::make_shared<transition_phys_fms_state>(self,phys_aircraft, time);
		}


        void transition_phys_fms_state::update(double time, double /*dt*/) 
        {
            if (!phys_aircraft_)
                return;

            double const transition_time = 5;

            double prediction = 30;
            geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*0.1));
            phys_aircraft_->go_to_pos(predict_pos, self_.get_fms_info()->get_state().orien());
            phys_aircraft_->set_air_cfg(self_.get_fms_info()->get_state().dyn_state.cfg);

            phys_aircraft_->update();

            auto physpos = phys_aircraft_->get_position();
            auto fmspos = self_.fms_pos();

            double t = cg::clamp(start_transition_time_, start_transition_time_ + transition_time, 0., 1.)(time);
            auto pos = cg::blend((geo_point_3)physpos.pos, (geo_point_3)fmspos.pos, t);
            auto orien = cg::blend(physpos.orien, fmspos.orien, t);
            double smooth = cg::blend(2., 25., t);

            self_.set_desired_nm_pos(pos);
            self_.set_desired_nm_orien(orien);
            self_.set_nm_angular_smooth(smooth);


            if (time >= start_transition_time_ + transition_time)
            {
                self_.switch_sync_state(create_fms_state(self_));
            }
        }


        void transition_phys_fms_state::on_zone_destroyed( size_t id )
        {
            if (phys_aircraft_->get_zone() == id)
                self_.switch_sync_state(boost::make_shared<none_state>(self_));
        }

    }
}
