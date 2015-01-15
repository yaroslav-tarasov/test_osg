#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace phys
{
    struct rigid_body;
    struct system;
    struct static_mesh;
    struct collision;
    struct static_convex;

    typedef polymorph_ptr<rigid_body>    rigid_body_ptr;
    typedef polymorph_ptr<system>        system_ptr;
    typedef polymorph_ptr<static_mesh>   static_mesh_ptr;
    typedef polymorph_ptr<static_convex> static_convex_ptr;
    typedef polymorph_ptr<collision> collision_ptr;

    namespace ray_cast_vehicle
    {
        struct info;
        struct control;

        typedef polymorph_ptr<info> info_ptr;
        typedef polymorph_ptr<control> control_ptr;
    }

    namespace aircraft
    {
        struct info;
        struct control;

        typedef polymorph_ptr<info> info_ptr;
        typedef polymorph_ptr<control> control_ptr;
    }
}


