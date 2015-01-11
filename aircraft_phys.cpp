#include "stdafx.h"
#include <btBulletDynamicsCommon.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>
#include "bi/bullet_helpers.h"
#include "aircraft_phys.h"
#include "../find_node_visitor.h"


namespace phys
{
	namespace aircraft
	{

    impl::impl(system_impl_ptr sys,/*compound_sensor_t const* s,*/compound_shape_proxy& s, params_t const& params, decart_position const& pos)
        : rigid_body_user_info_t(rb_aircraft)
		, sys_ (sys)
		, chassis_    (sys->dynamics_world())
		, raycast_veh_(sys->dynamics_world())
        , chassis_shape_(s)
		, params_(params)
    {
		tuning_.m_maxSuspensionTravelCm = 500;


		btTransform  tr;
		tr.setIdentity();
		btVector3 aabbMin,aabbMax;
		chassis_shape_.get()->getAabb(tr,aabbMin,aabbMax);

		btScalar dxx = btScalar(params_.wingspan / 2);
		btScalar dyy = btScalar(params_.length / 2);
		btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
		btScalar m12 = btScalar((params_.mass) /12);
		btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

		btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(params_.mass), NULL, &*chassis_shape_.get(), inertia);
		chassis_.reset(std::make_shared<btRigidBody>(chassis_construction_info));

		// FIXME TODO
		//chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
		//chassis_->setLinearVelocity(to_bullet_vector3(pos.dpos));

		//chassis_->setDamping(0.05f, 0.5f);
		chassis_->setRestitution(0.1f);
		//chassis_->setActivationState(DISABLE_DEACTIVATION);
		chassis_->setFriction(0.3f);

		raycast_veh_.reset(std::make_shared<btRaycastVehicle>(tuning_, &*chassis_.get(), &*sys->vehicle_raycaster()));

		raycast_veh_->setCoordinateSystem(0,2,1);


		sys_->dynamics_world()->addAction(this);

		sys_->register_rigid_body(this);

		chassis_->setUserPointer(this);
    }

	void impl::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
	{

	}

	void impl::debugDraw(btIDebugDraw* debugDrawer)
	{
		if (raycast_veh_->getNumWheels() == 0)
			return;

		raycast_veh_->debugDraw(debugDrawer);
	}

#if 0
	void aircraft::debugDraw(btIDebugDraw* debugDrawer)
	{
		if (raycast_veh_->getNumWheels() == 0)
			return;

		bt_collision_shape_ptr wheel_shape = bt_collision_shape_ptr(new btCylinderShapeX(to_bullet_vector3(point_3(0.3, raycast_veh_->getWheelInfo(0).m_wheelsRadius, raycast_veh_->getWheelInfo(0).m_wheelsRadius))));

		btVector3 aabbMin,aabbMax;
		sys_->dynamics_world()->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

		aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
		aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);

		for (int i=0;i<raycast_veh_->getNumWheels();i++)
		{
			//synchronize the wheels with the (interpolated) chassis worldtransform
			raycast_veh_->updateWheelTransform(i,false);
			
			osg::Matrix tr = from_bullet_transform(raycast_veh_->getWheelInfo(i).m_worldTransform);
			///dynamic_cast<debug_drawer_impl *>(debugDrawer)->draw_shape(tr, &*wheel_shape, cg::color_blue(), btIDebugDraw::DBG_DrawWireframe, aabbMin, aabbMax);
		}
	}
#endif

	size_t impl::add_wheel( double /*mass*/, double /*width*/, double radius, point_3 const& offset, cpr const & /*orien*/, bool /*has_damper*/, bool is_front )
	{
		point_3 connection_point = offset;

		//connection_point.z() += 1;

		btWheelInfo& info = raycast_veh_->addWheel(to_bullet_vector3(connection_point),btVector3(0,0,-1),btVector3(1,0,0), 1.0f,btScalar(radius),tuning_,is_front);
		info.m_suspensionStiffness = 20.f;
		info.m_wheelsDampingRelaxation = 2.3f;
		info.m_wheelsDampingCompression = 4.4f;
		info.m_frictionSlip = 10.;
		info.m_rollInfluence = 0.1f;

		wheels_ids_.push_back(raycast_veh_->getNumWheels()-1);
		return wheels_ids_.size()-1;
	}

	void impl::set_steer   (double steer)
	{
		for (int i=0;i<raycast_veh_->getNumWheels();i++)
		{
			btWheelInfo& info = raycast_veh_->getWheelInfo(i);
			if (info.m_bIsFrontWheel)
				raycast_veh_->setSteeringValue(btScalar(-cg::grad2rad() * steer), i);
		}

		if (!cg::eq(steer_, steer))
			chassis_->activate();

		steer_ = steer;
	}

	void impl::set_brake   (double brake)
	{
		for (int i=0;i<raycast_veh_->getNumWheels();i++)
		{
			raycast_veh_->setBrake(btScalar(brake * 10),i);
		}
	}

	bt_rigid_body_ptr impl::get_body() const
	{
		return chassis_.get();
	}

	void impl::pre_update(double /*dt*/)
	{
		//has_chassis_contact_ = false;
		//body_contact_points_.clear();
		//body_contacts_.clear();

		raycast_veh_.activate(chassis_->isActive());
	}

	void impl::has_contact(rigid_body_user_info_t const* /*other*/, osg::Vec3 const& local_point, osg::Vec3 const& vel)
	{
		//has_chassis_contact_ = true;
		//size_t id = body_contact_points_.insert(local_point).first;

		//if (!body_contacts_.valid(id))
		//	body_contacts_.insert(id, contact_t(vel));
		//else
		//{
		//	body_contacts_[id].sum_vel += vel;
		//	body_contacts_[id].count++;
		//}
	}
}

}
