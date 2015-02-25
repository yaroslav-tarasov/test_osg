#pragma once

#include "sync_fsm.h"
#include "../phys_aircraft.h"

namespace aircraft
{
namespace sync_fsm
{
    struct phys_state : state_t
    {
        phys_state(self_t &self, phys_aircraft_ptr phys_aircraft, geo_base_3 const& base)
            : self_(self)
            , on_ground_(false)
            , phys_aircraft_(phys_aircraft)
            , base_(base)
        {
            self.set_nm_angular_smooth(2);
            self_.set_phys_aircraft(phys_aircraft_);
        }

        ~phys_state()
        {
        }

        void deinit()
        {
            if (self_.get_shassis())
                self_.get_shassis()->freeze();
            self_.set_phys_aircraft(nullptr);
        }

        void update(double /*time*/, double dt);
        void on_zone_destroyed( size_t id );
        void on_fast_session( bool fast );


    private:
        void sync_wheels(double dt);

    private:
        self_t &self_;
        geo_base_3 base_;
        size_t zone_;
        phys_aircraft_ptr phys_aircraft_;
        bool on_ground_;

    public:
        static const double phys_height;
    };

}
}