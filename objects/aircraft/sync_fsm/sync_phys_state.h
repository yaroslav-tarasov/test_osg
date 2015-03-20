#pragma once

#include "sync_fsm.h"
#include "../phys_aircraft.h"

namespace aircraft
{
namespace sync_fsm
{
    enum phys_state_t {ORIGINAL, TEST_NEW};
    
    sync_fsm::state_ptr create_sync_phys_state(phys_state_t type,self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base);
    
    inline static sync_fsm::state_ptr create_sync_phys_state(self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
    {
             return create_sync_phys_state(TEST_NEW,self, phys_aircraft, base);
    };

}
}