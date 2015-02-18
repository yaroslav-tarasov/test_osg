#include "static_convex.h"
#include "rigid_body_info.h"

namespace phys
{
    static_convex_impl::static_convex_impl(system_impl_ptr sys, sensor_ptr s, point_3 const& pos, quaternion const& orien)
        : sys_(sys)
        , shape_(get_sensor_convex(s))
        , body_(sys->dynamics_world(), boost::make_shared<btRigidBody>(0.f, (btMotionState*)0, shape_.get(), btVector3()))
    {
        body_.get()->setRestitution(0.01f);
        body_.get()->setCenterOfMassTransform(to_bullet_transform(pos, orien));
        body_.get()->setFriction(0.99f);
        body_.get()->setActivationState(DISABLE_SIMULATION);
        body_.get()->setUserPointer(new rigid_body_user_info_t(rb_static_convex));
        body_.get()->setCollisionFlags(body_.get()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    }

    static_convex_impl::~static_convex_impl()
    {
    }

}
