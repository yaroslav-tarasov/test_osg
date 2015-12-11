#include "stdafx.h"
#include "precompiled_objects.h"

#include "sync_pl_transition_phys_fms.h"
#include "sync_pl_fms_state.h"
#include "sync_pl_none_state.h"

namespace aircraft_physless
{
    namespace sync_fsm
    {

        void transition_phys_fms_state::update(double time, double /*dt*/) 
        {
            if (!phys_aircraft_)
                return;

            double const transition_time = 5;

            double prediction = 30;
            // geo_base_3 predict_pos = geo_base_3(aircraft_fms::model_info_ptr(self_.get_fms_info())->prediction(prediction*0.1));
            // Физическое альтерэго должно двигаться исключительно под внешним воздействием 
            geo_base_3 predict_pos = self_.get_root_pos().pos;

            phys_aircraft_->go_to_pos(predict_pos, self_.get_root_pos().orien );
            // phys_aircraft_->set_air_cfg(self_.get_fms_info()->get_state().dyn_state.cfg);

            phys_aircraft_->update();

            //self_.set_desired_nm_pos(pos);
            //self_.set_desired_nm_orien(orien);
            //self_.set_nm_angular_smooth(smooth);


            if (time >= start_transition_time_ + transition_time)
            {
                self_.switch_sync_state(boost::make_shared<fms_state>(self_));
            }
        }


        void transition_phys_fms_state::on_zone_destroyed( size_t id )
        {
            if (phys_aircraft_->get_zone() == id)
                self_.switch_sync_state(boost::make_shared<none_state>(self_));
        }

    }
}
