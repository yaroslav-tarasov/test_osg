#pragma once

#include "phys/phys_sys_fwd.h"
#include "objects/rocket_flare.h"

namespace phys
{
    namespace rocket_flare
    {
		struct params_t
		{
			params_t()
				: mass(0.0)
			{}

			double mass;
		};  

        struct contact_info_t
        {
            contact_info_t() {}
            contact_info_t( cg::point_3 const& offset, cg::point_3 const& vel )
                : vel(vel), offset(offset)
            {}

            cg::point_3 vel;
            cg::point_3 offset;
        };
        
        
        struct info
        {
            virtual ~info() {}
            virtual decart_position get_position() const = 0;
            virtual std::vector<contact_info_t> get_body_contacts() const = 0;
        };

        struct control
        {
            virtual ~control() {}
			virtual void   set_wind            (cg::point_3 const& wind)    = 0;
			virtual void   apply_force         (point_3 const& f)           = 0;
			virtual void   set_linear_velocity (point_3 const& f)           = 0;
			virtual void   set_angular_velocity(point_3 const& a)           = 0;
			virtual void   set_position        (const decart_position& pos) = 0;
        };

    }
}
