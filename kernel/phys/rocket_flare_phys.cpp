#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include "bullet_helpers.h"

#include "rocket_flare.h"
#include "rigid_body_info.h"
#include "cpp_utils/polymorph_ptr.h"
#include "phys_sys.h"
#include "phys_sys_common.h"

#include "rocket_flare_phys.h"



namespace phys
{


namespace {

    btRigidBody*	create_rigid_body(btDiscreteDynamicsWorld* dW,float mass, const btTransform& startTransform, btCollisionShape* shape,  const btVector4& color = btVector4(1, 0, 0, 1))
    {
        btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            shape->calculateLocalInertia(mass, localInertia);

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

        btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

        btRigidBody* body = new btRigidBody(cInfo);
        //body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
        btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
        body->setWorldTransform(startTransform);
#endif//

        body->setUserIndex(-1);
        dW->addRigidBody(body);
        return body;
    }

	btCompoundShape*  create_compound_shape()
	{
		btCompoundShape* cs = new btCompoundShape;
		cs->addChildShape(btTransform(),new btBoxShape(btVector3(.6,.6,.6)));
		return cs;
	}


}



	namespace rocket_flare
	{

    impl::impl(system_impl_ptr sys,compound_sensor_ptr s,params_t const& params, decart_position const& pos)
        : bt_body_user_info_t(rb_simple)
		, sys_                  (sys)
		, chassis_              (sys->dynamics_world())
        , chassis_shape_        (s?compound_sensor_impl_ptr(s)->cs_: create_compound_shape())
		, params_               (params)
        , has_chassis_contact_  (false)
        , body_contact_points_  (1.5)
    {

		btTransform  tr;
		tr.setIdentity();
		btVector3 aabbMin,aabbMax;

#if 1
        FIXME(chassis_shape_)
        chassis_shape_->getAabb(tr,aabbMin,aabbMax);
       
        const float  fake_mass = 20;

		btScalar dxx = btScalar((aabbMax.x() - aabbMin.x()) / 2);
		btScalar dyy = btScalar((aabbMax.y() - aabbMin.y()) / 2);
		btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
		btScalar m12 = btScalar((fake_mass) /12);
		btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

        btDefaultMotionState* motionState = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(fake_mass), /*NULL*//*new custom_ms*/motionState,  new btBoxShape(btVector3(.6,.6,.6))/*chassis_shape_*//*&*chassis_shape_.get()*/, inertia);
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

        //sys_->dynamics_world()->addAction(this);

		sys_->register_rigid_body(this);

#if 1
		chassis_->setUserPointer(this);
#endif
    }

	impl::~impl()
	{
		chassis_.reset();
		sys_->unregister_rigid_body(this);
		//sys_->dynamics_world()->removeAction(this);
	}

	void impl::update(double dt)
	{

	}

	void impl::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
	{
        update(deltaTimeStep);
	}

    void impl::debugDraw(btIDebugDraw* debugDrawer)
    {
    }

    bool impl::has_contact() const
    {
        if (has_chassis_contact_)
            return true;

        return false;
    }
    
    void  impl::set_linear_velocity (point_3 const& v)
    {
        chassis_->setLinearVelocity(to_bullet_vector3(v));
    }
    
    void  impl::set_angular_velocity (point_3 const& a)
    {
        chassis_->setAngularVelocity(to_bullet_vector3(a));
    }

    void impl::apply_force (point_3 const& f)
    {
        chassis_->applyCentralForce(to_bullet_vector3(f));
    }

    void impl::set_wind    (point_3 const& wind)
    {
        wind_ = wind;
    }

	bt_rigid_body_ptr impl::get_body() const
	{
		return chassis_.get();
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
        return from_bullet_position(&*chassis_.get());
    }
	
	void impl::set_position(const decart_position& pos)
	{
		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
	}
    
     params_t const& impl::params() const
    {
        return params_;
    }


}

}
