#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>
#include "bi/bullet_helpers.h"

#include "ray_cast_vehicle.h"
//#include "debug_drawer.h"
#include "bi/phys_sys_common.h"



namespace phys
{
namespace ray_cast_vehicle
{
    impl::impl(system_impl_ptr sys, double mass, compound_sensor_ptr s,/*sensor_ptr s,*/ decart_position const& pos)
        : sys_(sys)
        , tow_rod_(sys->dynamics_world())
        , tow_constraint_self_(sys->dynamics_world())
        , tow_constraint_tow_(sys->dynamics_world())
        , chassis_(sys->dynamics_world())
        , raycast_veh_(sys->dynamics_world())
        , steer_(0)
        , chassis_shape_(compound_sensor_impl_ptr(s)->cs_)
    {
        tuning_.m_maxSuspensionTravelCm = 50;

        //chassis_shape_ = get_sensor_convex(s);
        
#if 0   // TODO FIXME ЁЁЁ чего?
        bt_collision_shape_ptr         convex_shape = get_sensor_convex(s);

        btVector3 inertia;
        convex_shape->calculateLocalInertia(btScalar(mass), inertia);
#else
        btVector3 inertia;
        chassis_shape_->calculateLocalInertia(btScalar(mass), inertia);
       
        btTransform  tr;
        tr.setIdentity();
        btVector3 aabbMin,aabbMax;
        chassis_shape_.get()->getAabb(tr,aabbMin,aabbMax);

        btScalar dxx = btScalar((aabbMax.x() - aabbMin.x()) / 2);
        btScalar dyy = btScalar((aabbMax.y() - aabbMin.y()) / 2);
        btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
        btScalar m12 = btScalar((mass) /12);
        inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

#endif

        btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(mass), new custom_ms, &*chassis_shape_, inertia);
        chassis_.reset(boost::make_shared<btRigidBody>(chassis_construction_info));

        chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));

        chassis_->setRestitution(0.1f);
        //chassis_->setActivationState(DISABLE_DEACTIVATION);
        //chassis_->setFriction(0.1);

        raycast_veh_.reset(boost::make_shared<btRaycastVehicle>(tuning_, &*chassis_.get(), &*sys->vehicle_raycaster()));


        raycast_veh_->setCoordinateSystem(0,2,1);

        sys_->register_rigid_body(this);
        
        sys_->dynamics_world()->addAction(this);
    }

    impl::~impl()
    {
        sys_->unregister_rigid_body(this);
        sys_->dynamics_world()->removeAction(this);
    }

    bt_rigid_body_ptr impl::get_body() const
    {
        return chassis_.get();
    }

    void impl::pre_update(double /*dt*/)
    {
        raycast_veh_.activate(chassis_->isActive());
    }

    void impl::updateAction( btCollisionWorld* /*collisionWorld*/, btScalar /*deltaTimeStep*/)
    {
    }

#if 0
    void impl::debugDraw(btIDebugDraw* debugDrawer)
    {
        if (raycast_veh_->getNumWheels() == 0)
            return;

        bt_collision_shape_ptr wheel_shape = bt_collision_shape_ptr(new btCylinderShapeX(to_bullet_vector3(cg::point_3(0.3, raycast_veh_->getWheelInfo(0).m_wheelsRadius, raycast_veh_->getWheelInfo(0).m_wheelsRadius))));

        btVector3 aabbMin,aabbMax;
        sys_->dynamics_world()->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

        aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
        aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);

        for (int i=0;i<raycast_veh_->getNumWheels();i++)
        {
            //synchronize the wheels with the (interpolated) chassis worldtransform
            raycast_veh_->updateWheelTransform(i,false);

            cg::transform_4 tr = from_bullet_transform(raycast_veh_->getWheelInfo(i).m_worldTransform);
            dynamic_cast<debug_drawer_impl *>(debugDrawer)->draw_shape(tr, &*wheel_shape, cg::color_blue(), btIDebugDraw::DBG_DrawWireframe, aabbMin, aabbMax);
        }
    }
#else
    void impl::debugDraw(btIDebugDraw* debugDrawer)
    {
        if (raycast_veh_->getNumWheels() == 0)
            return;

        raycast_veh_->debugDraw(debugDrawer);
    }
#endif

    void impl::add_wheel( double /*mass*/, double /*width*/, double radius, cg::point_3 const& offset, cg::cpr const & /*orien*/, bool /*has_damper*/ )
    {
        bool isFrontWheel = offset.y > 0;

        cg::point_3 connection_point = offset;
        //connection_point.z += 1;

        tuning_.m_maxSuspensionForce = 60000000;

        btWheelInfo& info = raycast_veh_->addWheel(to_bullet_vector3(connection_point),btVector3(0,0,-1),btVector3(1,0,0),0/*1.1f*/,btScalar(radius),tuning_,isFrontWheel);
        info.m_suspensionStiffness = 20.f;
        info.m_wheelsDampingRelaxation = 2.3f;
        info.m_wheelsDampingCompression = 4.4f;
        info.m_frictionSlip = 1.;
        info.m_rollInfluence = 0.1f;
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
            raycast_veh_->setBrake(btScalar(brake * 100),i);
        }
    }

    void impl::set_thrust  (double thrust)
    {
        for (int i=0;i<raycast_veh_->getNumWheels();i++)
        {
            raycast_veh_->applyEngineForce(btScalar(1000 * thrust),i);
        }
        
        if (!cg::eq_zero(thrust))
            chassis_->activate();
    }

    decart_position impl::get_position() const
    {
        return from_bullet_position(&*chassis_.get());
    }

    decart_position impl::get_wheel_position( size_t i ) const
    {
        //raycast_veh_->updateWheelTransform(i,false);

        cg::transform_4 tr = from_bullet_transform(raycast_veh_->getWheelInfo(i).m_worldTransform);

        decart_position pos;
        pos.pos = tr.translation();
        pos.orien = tr.rotation().cpr();

        return pos;
    }

    double impl::get_tow_rod_course() const
    {
        //Assert(tow_rod_);
        if (tow_rod_)
        {
            cg::quaternion q = from_bullet_quaternion(tow_rod_.get()->getWorldTransform().getRotation());
            return q.get_course();
        }

        return 0;
    }

    void impl::set_course_hard(double course)
    {
        decart_position  pos=  get_position();
        pos.orien = cg::cpr(course, 0, 0);
        chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
    }

    void impl::reset_suspension()
    {
        for (int i=0;i<raycast_veh_->getNumWheels();i++)
        {
            raycast_veh_->resetSuspension();
            raycast_veh_->updateWheelTransform(i,false);
        }
    }

    void impl::set_tow(rigid_body_ptr rb, cg::point_3 const& self_offset, cg::point_3 const& offset)
    {
        tow_ = rigid_body_impl_ptr(rb)->get_body();

        cg::transform_4 self_transform = from_bullet_transform(chassis_->getWorldTransform());
        cg::point_3 self_point = self_transform * self_offset;

        cg::transform_4 tow_transform = from_bullet_transform(tow_->getWorldTransform());
        cg::point_3 tow_point = tow_transform * offset;

        double half_dist = cg::distance(self_point, tow_point) * 0.5;

        // tow rod
//        tow_rod_shape_ = bt_collision_shape_ptr(new btCylinderShape(to_bullet_vector3(point_3(0.1, half_dist, 0.1))));
        tow_rod_shape_ = bt_collision_shape_ptr(new btEmptyShape());

        double const rod_mass = 100;
        double const rod_radius = 0.1;

        btVector3 inertia(btScalar(rod_mass*(3*cg::sqr(rod_radius)+ cg::sqr(2*half_dist))/12), 
                          btScalar(rod_mass*cg::sqr(rod_radius)/2),
                          btScalar(rod_mass*(3*cg::sqr(rod_radius)+ cg::sqr(2*half_dist))/12));

        //tow_rod_shape_->calculateLocalInertia(10, inertia);

        btRigidBody::btRigidBodyConstructionInfo tow_rod_construction_info(btScalar(rod_mass), NULL, &*tow_rod_shape_, inertia);
        tow_rod_.reset(boost::make_shared<btRigidBody>(tow_rod_construction_info));
        tow_rod_.get()->setCollisionFlags(tow_rod_.get()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);


        cg::point_3 rod_pos = (self_point + tow_point) / 2;
        cg::polar_point_3 rod_fwd = self_point - tow_point;


        tow_rod_.get()->setCenterOfMassTransform(to_bullet_transform(rod_pos, cg::cpr(rod_fwd.course, rod_fwd.pitch, 0)));

        tow_constraint_self_.reset(boost::make_shared<btGeneric6DofConstraint>(*tow_rod_.get(), *chassis_.get(),  
                                        to_bullet_transform(cg::point_3(0, half_dist, 0), cg::cpr()), 
                                        to_bullet_transform(self_offset, cg::cpr(0,0,0)), true));

        tow_constraint_self_.get()->setLinearLowerLimit(to_bullet_vector3(cg::point_3(0, 0, 0)));
        tow_constraint_self_.get()->setLinearUpperLimit(to_bullet_vector3(cg::point_3(0, 0, 0)));
        tow_constraint_self_.get()->setAngularLowerLimit(btVector3(SIMD_PI, SIMD_PI, btScalar(-80*cg::grad2rad())));
        tow_constraint_self_.get()->setAngularUpperLimit(btVector3(-SIMD_PI, -SIMD_PI, btScalar(80*cg::grad2rad())));

//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_ERP, 0.1, 0);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_ERP, 0.1, 1);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_ERP, 0.1, 2);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_ERP, 0.1, 3);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_ERP, 0.1, 4);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_ERP, 0.1, 5);
// 
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_CFM, 0.5, 3);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_CFM, 0.5, 4);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_CFM, 0.5, 5);

//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_CFM, 1., 3);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_CFM, 1., 4);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_CFM, 1., 5);

//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_CFM, 0.5, 0);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_CFM, 0.5, 1);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_STOP_CFM, 0.5, 2);

//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_CFM, 1., 0);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_CFM, 1., 1);
//         tow_constraint_self_.get()->setParam(BT_CONSTRAINT_CFM, 1., 2);

        tow_constraint_tow_.reset(boost::make_shared<btGeneric6DofConstraint>(*tow_rod_.get(), *tow_, 
                                        to_bullet_transform(cg::point_3(0, -half_dist, 0), cg::cpr()), 
                                        to_bullet_transform(offset, cg::cpr(0,0,0)), true));

        tow_constraint_tow_.get()->setLinearLowerLimit(to_bullet_vector3(cg::point_3(0, 0, 0)));
        tow_constraint_tow_.get()->setLinearUpperLimit(to_bullet_vector3(cg::point_3( 0, 0, 0)));
        tow_constraint_tow_.get()->setAngularLowerLimit(btVector3(SIMD_PI, SIMD_PI, SIMD_PI));
        tow_constraint_tow_.get()->setAngularUpperLimit(btVector3(-SIMD_PI, -SIMD_PI, -SIMD_PI));
    }

    void impl::reset_tow()
    {         
        tow_constraint_self_.reset();
        tow_constraint_tow_.reset();
        tow_rod_.reset();
        tow_rod_shape_.reset();
        tow_.reset();
    }


}

}
