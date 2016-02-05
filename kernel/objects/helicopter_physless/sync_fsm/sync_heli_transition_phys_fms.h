#pragma once

#include "sync_heli_fsm.h"
#include "../phys_helicopter_phl.h"

namespace helicopter_physless
{

namespace sync_fsm
{
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

}
}