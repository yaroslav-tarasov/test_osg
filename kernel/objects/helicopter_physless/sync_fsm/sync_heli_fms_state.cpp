#include "stdafx.h"
#include "precompiled_objects.h"

#include "sync_heli_fms_state.h"
#include "sync_heli_none_state.h"
#include "sync_heli_phys_state.h"
#include "sync_heli_transition_fms_phys.h"

namespace helicopter_physless
{
namespace sync_fsm
{
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
        //if (!self_.is_fast_session() && !phys_aircraft_failed_ && fms_pos.pos.height < /*phys_state::*/phys_height() - 1 && self_.phys_control()->get_zone(fms_pos.pos))
        //{
        //    optional<size_t> phys_zone = self_.phys_control()->get_zone(fms_pos.pos);
        //    if (phys_zone)
        //    {
        //        auto base = self_.phys_control()->get_base(*phys_zone);
        //        auto phys_sys = self_.phys_control()->get_system(*phys_zone);
        //                                                                                      // FIXME we need it
        //        phys_aircraft_ptr phys_aircraft = phys_aircraft_impl::create(base, phys_sys, /*self_.get_meteo_cursor(),*/ 
        //            self_.get_nodes_manager(), self_.fms_pos(), 
        //            *self_.get_fms_info()->fsettings(), self_.get_shassis(), *phys_zone);

        //        if (phys_aircraft)
        //        {
        //            if (!cg::eq_zero(fms_pos.pos.height))
        //                self_.switch_sync_state(boost::make_shared<transition_fms_phys_state>(self_, phys_aircraft, base, time));
        //            else
        //                self_.switch_sync_state(/*boost::make_shared<phys_state>*/create_sync_phys_state(self_, phys_aircraft, base));

        //        }
        //    }
        //}
        //else
        if (cg::distance2d(airprt->pos(), fms_pos.pos) > none_state::sync_none_dist)
        {
            self_.switch_sync_state(boost::make_shared<none_state>(self_));
        }
        else
        {
            optional<size_t> phys_zone = self_.phys_control()->get_zone(fms_pos.pos);
            if (phys_zone)
            {
                auto base = self_.phys_control()->get_base(*phys_zone);
                auto phys_sys = self_.phys_control()->get_system(*phys_zone);

                phys_aircraft_ptr phys_aircraft = phys_heli_impl::create(base, phys_sys, /*self_.get_meteo_cursor(),*/ 
                            self_.get_nodes_manager(), self_.fms_pos(), 
                            *self_./*get_fms_info()->*/fsettings(), self_.get_shassis(), *phys_zone);
            

                FIXME(И гиде взять координаты)
                self_.switch_sync_state(create_sync_phys_state(self_, phys_aircraft, /*base*/::get_base()));
            }
        }



    }
}
}
