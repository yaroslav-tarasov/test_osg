#include "stdafx.h"
#include "precompiled_objects.h"

#include "sync_heli_none_state.h"
#include "sync_heli_fms_state.h"

namespace helicopter_physless
{
namespace sync_fsm
{
    void none_state::update(double /*time*/, double /*dt*/) 
    {                                                                   
        if (self_.get_airports_manager())
        {
            auto fms_pos = self_.fms_pos();

            airport::info_ptr air = self_.get_airports_manager()->find_closest_airport(fms_pos.pos);
            if (air)
            {
                if (cg::distance2d(air->pos(), fms_pos.pos) < sync_none_dist)
                    self_.switch_sync_state(boost::make_shared<fms_state>(self_));

            }
        }
    }
}
}