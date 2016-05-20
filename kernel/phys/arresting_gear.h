#pragma once

#include "phys/phys_sys_fwd.h"
#include "objects/arresting_gear.h"

namespace phys
{
    namespace arresting_gear
    {
        struct params_t
        {
            typedef std::pair<cg::point_3f,cg::point_3f> rope_t;

            params_t()
            {}

            vector<rope_t> ropes;
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
            virtual std::vector<::arresting_gear::rope_state_t>    get_ropes_info() const = 0; 
        };

        struct control
        {
            virtual ~control() {}
            virtual void   set_wind            (cg::point_3 const& wind)    = 0;
            virtual void   apply_force         (point_3 const& f)           = 0;
            virtual void   set_linear_velocity (point_3 const& f)           = 0;
            virtual void   set_angular_velocity(point_3 const& a)           = 0;
            virtual void   set_position        (const decart_position& pos) = 0;
            virtual void   append_anchor       (rigid_body_ptr body, cg::point_3 const& pos)     = 0;
            virtual void   release_anchor      (rigid_body_ptr body)     = 0;
        };

    }
}
