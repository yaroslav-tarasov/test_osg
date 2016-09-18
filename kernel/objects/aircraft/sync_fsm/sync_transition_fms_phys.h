#pragma once

#include "sync_fsm.h"
#include "../phys_aircraft.h"

namespace aircraft
{
namespace sync_fsm
{
	  	sync_fsm::state_ptr create_transition_fms_phys_state(self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base, double time);

}
}