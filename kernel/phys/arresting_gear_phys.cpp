#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include "bullet_helpers.h"

#include "arresting_gear.h"
#include "rigid_body_info.h"
#include "cpp_utils/polymorph_ptr.h"
#include "phys_sys.h"
#include "phys_sys_common.h"

#include "arresting_gear_phys.h"



namespace phys
{
	namespace arresting_gear
	{

    impl::impl(system_impl_ptr sys,compound_sensor_ptr s,params_t const& params, decart_position const& pos)
        : bt_body_user_info_t(bt_soft_body)
		, sys_                  (sys)
#if 0
		, chassis_              (sys->dynamics_world())
        , chassis_shape_        (compound_sensor_impl_ptr(s)->cs_)
#endif
		, rope_					(sys->dynamics_world())
		, params_               (params)
        , has_chassis_contact_  (false)
        , body_contact_points_  (1.5)
    {

		btTransform  tr;
		tr.setIdentity();
		btVector3 aabbMin,aabbMax;

#if 0
        chassis_shape_->getAabb(tr,aabbMin,aabbMax);

        const float  fake_mass = 20;

		btScalar dxx = btScalar((aabbMax.x() - aabbMin.x()) / 2);// btScalar(params_.wingspan / 2);
		btScalar dyy = btScalar((aabbMax.y() - aabbMin.y()) / 2);// btScalar(params_.length / 2);
		btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
		btScalar m12 = btScalar((fake_mass) /12);
		btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

        btDefaultMotionState* motionState = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(fake_mass), /*NULL*//*new custom_ms*/motionState, chassis_shape_/*&*chassis_shape_.get()*/, inertia);
		chassis_.reset(boost::make_shared<btRigidBody>(chassis_construction_info));

		// FIXME TODO
		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
#if 0
		chassis_->setLinearVelocity(to_bullet_vector3(pos.dpos));
#endif
		//chassis_->setDamping(0.05f, 0.5f);
		chassis_->setRestitution(0.1f);
		//chassis_->setActivationState(DISABLE_DEACTIVATION);
		chassis_->setFriction(0.3f);
#endif

		// sys_->dynamics_world()->addAction(this);

		sys_->register_rigid_body(this);

#if 0
		chassis_->setUserPointer(this);
#endif
    }

	void impl::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
	{
        //update_aerodynamics(deltaTimeStep);
	}

    void impl::debugDraw(btIDebugDraw* debugDrawer)
    {
#if 0
        if (raycast_veh_->getNumWheels() == 0)
            return;

        raycast_veh_->debugDraw(debugDrawer);
#endif
    }

    bool impl::has_contact() const
    {
        if (has_chassis_contact_)
            return true;

        return false;
    }
    
    void  impl::set_linear_velocity (point_3 const& v)
    {
#if 0
        chassis_->setLinearVelocity(to_bullet_vector3(v));
#endif
    }
    
    void  impl::set_angular_velocity (point_3 const& a)
    {
#if 0
        chassis_->setAngularVelocity(to_bullet_vector3(a));
#endif
    }

    void impl::apply_force (point_3 const& f)
    {
#if 0
        chassis_->applyCentralForce(to_bullet_vector3(f));
#endif
    }

    void impl::set_wind    (point_3 const& wind)
    {
        wind_ = wind;
    }

	bt_rigid_body_ptr impl::get_body() const
	{
		return bt_rigid_body_ptr(); //chassis_.get();
	}

	void impl::pre_update(double /*dt*/)
	{
		has_chassis_contact_ = false;
		body_contact_points_.clear();
		body_contacts_.clear();
	}

    void impl::has_contact(bt_body_user_info_t const* /*other*/, point_3 const& local_point, point_3 const& vel)
    {
        has_chassis_contact_ = true;
        size_t id = body_contact_points_.insert(local_point).first;

        if (!body_contacts_.valid(id))
            body_contacts_.insert(id, contact_t(vel));
        else
        {
            body_contacts_[id].sum_vel += vel;
            body_contacts_[id].count++;
        }
    }

    std::vector<contact_info_t> impl::get_body_contacts() const
    {
        std::vector<contact_info_t> res;
        for (auto it = body_contact_points_.begin(); it != body_contact_points_.end(); ++it)
        {
                point_3 vel = body_contacts_[it.id()].sum_vel / body_contacts_[it.id()].count;
            res.push_back(contact_info_t(*it, vel));
        }

        return res;
    }

    decart_position impl::get_position() const
    {
        return decart_position(); //from_bullet_position(&*chassis_.get());
    }
	
	void impl::set_position(const decart_position& pos)
	{
#if 0
		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
#endif
	}

    params_t const& impl::params() const
    {
        return params_;
    }

}

}
