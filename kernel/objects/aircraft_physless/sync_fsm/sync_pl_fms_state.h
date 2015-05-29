#pragma once

#include "sync_pl_fsm.h"

namespace aircraft_physless
{
namespace sync_fsm
{
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


    private:
        self_t &self_;
        bool phys_aircraft_failed_;
    };
}
}