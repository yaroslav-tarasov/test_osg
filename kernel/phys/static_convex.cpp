#include "stdafx.h"
#include <btBulletDynamicsCommon.h>
#include "bullet_helpers.h"
#include "static_convex.h"
#include "rigid_body_info.h"
#include "phys/phys_sys.h"

using namespace phys;

FIXME(Не то и не там)

inline bt_collision_shape_ptr get_sensor_convex( sensor_ptr s )
{
	btAlignedObjectArray<btVector3> points;

	//for (size_t i = 0; i < s->chunks_count(); ++i)
	//{
	//	for (size_t j = 0; j < s->triangles_count(i); ++j)
	//	{
	//		cg::triangle_3f tr = s->triangle(i, j);
	//		for (size_t k = 0; k < 3; ++k) 
	//		{
	//			if (cg::norm(tr[k]) > 50)
	//				continue;
	//			points.push_back(to_bullet_vector3(tr[k]));
	//		}
	//	}
	//}

	return bt_collision_shape_ptr(new btConvexHullShape((btScalar *)&points[0], points.size()));
}

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
