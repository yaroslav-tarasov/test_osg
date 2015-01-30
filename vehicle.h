#pragma once

#include "phys_sys_fwd.h"

namespace phys
{
    namespace ray_cast_vehicle
    {
        struct info
        {
            virtual ~info() {}
            virtual void add_wheel( double mass, double width, double radius, cg::point_3 const& offset, cg::cpr const & orien, bool has_damper ) = 0;

            virtual void set_steer   (double steer) = 0;
            virtual void set_brake   (double brake) = 0;
            virtual void set_thrust  (double thrust) = 0;

            virtual decart_position get_position() const = 0;
            virtual decart_position get_wheel_position( size_t i ) const = 0;

            virtual double get_tow_rod_course() const = 0;
        };

        struct control
        {
            virtual ~control() {}

            virtual void set_tow(rigid_body_ptr rb, cg::point_3 const& self_offset, cg::point_3 const& offset) = 0;
            virtual void reset_tow() = 0;
            virtual void set_course_hard(double course) = 0;
            virtual void reset_suspension() = 0;
        };

    }
}