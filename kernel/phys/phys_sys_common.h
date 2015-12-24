#pragma  once

#include "rigid_body_info.h"

namespace phys
{

    struct rigid_body_impl : rigid_body
    {
        virtual bt_rigid_body_ptr get_body() const = 0;
        virtual void pre_update(double dt) = 0;
        virtual void has_contact(rigid_body_user_info_t const* other, cg::point_3 const& local_point, cg::point_3 const& vel) = 0;
    };

    struct system_impl
    {
        virtual ~system_impl() {}
        virtual bt_dynamics_world_ptr    dynamics_world() const = 0;
        virtual bt_vehicle_raycaster_ptr vehicle_raycaster() const = 0;
        virtual void register_rigid_body( rigid_body_impl * rb ) = 0;
        virtual void unregister_rigid_body( rigid_body_impl * rb ) = 0;
    };


    typedef polymorph_ptr<rigid_body_impl> rigid_body_impl_ptr;
    typedef polymorph_ptr<system_impl>     system_impl_ptr;
}