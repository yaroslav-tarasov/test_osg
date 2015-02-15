#pragma once

#include "phys_sys.h"

namespace phys
{
    struct static_convex_impl
        : static_convex
    {
        static_convex_impl(system_impl_ptr sys, sensor_ptr s, point_3 const& pos, quaternion const& orien);
        ~static_convex_impl();

    private:
        system_impl_ptr sys_;

        bt_collision_shape_ptr shape_;
        rigid_body_proxy       body_;
    };
}