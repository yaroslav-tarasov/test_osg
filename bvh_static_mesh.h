#pragma once

#include "phys_sys.h"
#include "bi/phys_sys_common.h"

namespace phys
{
    struct bvh_static_mesh 
        : static_mesh
    {
        bvh_static_mesh(system_impl_ptr sys, sensor_ptr s);
        ~bvh_static_mesh();

    private:
        system_impl_ptr sys_;
        btTriangleMesh mesh_;
        bt_collision_shape_ptr shape_;
        bt_rigid_body_ptr body_;
    };
}