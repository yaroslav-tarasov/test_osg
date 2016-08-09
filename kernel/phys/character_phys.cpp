#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include "bullet_helpers.h"

#include "character_phys.h"


namespace phys
{
	namespace character
	{

    impl::impl(system_impl_ptr sys,compound_sensor_ptr s,/*compound_shape_proxy& s,*/ params_t const& params, decart_position const& pos)
        : bt_body_user_info_t(rb_character)
		, sys_                  (sys)
		, chassis_              (sys->dynamics_world())
        , chassis_shape_        (compound_sensor_impl_ptr(s)->cs_)
		, params_               (params)
        , prev_attack_angle_    (0)
        , has_chassis_contact_  (false)
        , body_contact_points_  (1.5)
    {

		btTransform  tr;
		tr.setIdentity();
		btVector3 aabbMin,aabbMax;
#if 0
        chassis_shape_.get()->getAabb(tr,aabbMin,aabbMax);
#else
        chassis_shape_->getAabb(tr,aabbMin,aabbMax);
#endif

		btScalar dxx = 1.f; //btScalar((aabbMax.x() - aabbMin.x()) / 2);
		btScalar dyy = 1.f; //btScalar((aabbMax.y() - aabbMin.y()) / 2);
		btScalar dzz = 1.f; //btScalar((aabbMax.z() - aabbMin.z()) / 2);
		btScalar m12 = btScalar((params_.mass) /12);
		btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

        // btDefaultMotionState* motionState = new btDefaultMotionState(tr);
        FIXME(Вынести как-нибудь);

       
        btDefaultMotionState* motionState =
            new btDefaultMotionState(to_bullet_transform(pos.pos, pos.orien)/*btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0))*/);

		btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(params_.mass), motionState, chassis_shape_/*&*chassis_shape_.get()*/, inertia);
		chassis_.reset(phys::bt_rigid_body_ptr(boost::make_shared<btRigidBody>(chassis_construction_info)));
        
		// FIXME TODO

		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
		chassis_->setLinearVelocity(to_bullet_vector3(pos.dpos));

		// chassis_->setDamping(0.05f, 0.5f);
		chassis_->setRestitution(0.1f);
		//chassis_->setActivationState(DISABLE_SIMULATION);
		chassis_->setFriction(0.01f);

		// sys_->dynamics_world()->addAction(this);

		sys_->register_rigid_body(this);

		chassis_->setUserPointer(this);
    }

	impl::~impl()
	{
		chassis_.reset();
		sys_->unregister_rigid_body(this);
		//sys_->dynamics_world()->removeAction(this);
	}

	void impl::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
	{
        update_action(deltaTimeStep);
	}

    void impl::update_action(double dt)
    {
        using namespace cg;

        if (!chassis_->isActive())
            return;

    }

    void impl::activate(bool active)
    {
        chassis_->forceActivationState(active?DISABLE_DEACTIVATION:DISABLE_SIMULATION);
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

    double impl::Ixx() const
    {
        return 1. / chassis_->getInvInertiaDiagLocal().x();
    }

    double impl::Iyy() const
    {
        return 1. / chassis_->getInvInertiaDiagLocal().y();
    }

    double impl::Izz() const
    {
        return 1. / chassis_->getInvInertiaDiagLocal().z();
    }

    params_t const& impl::params() const
    {
        return params_;
    }

}

}
