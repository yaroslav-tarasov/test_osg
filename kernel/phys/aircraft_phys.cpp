#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include "bullet_helpers.h"

#include "aircraft_phys.h"


namespace phys
{
	namespace aircraft
	{

    impl::impl(system_impl_ptr sys,compound_sensor_ptr s,/*compound_shape_proxy& s,*/ params_t const& params, decart_position const& pos)
        : rigid_body_user_info_t(rb_aircraft)
		, sys_ (sys)
		, chassis_    (sys->dynamics_world())
		, raycast_veh_(sys->dynamics_world())
        , chassis_shape_(compound_sensor_impl_ptr(s)->cs_)
		, params_(params)
        , elevator_(0)
        , ailerons_(0)
        , rudder_(0)
        , thrust_(0)
        , steer_(0)
        , prev_attack_angle_(0)
        , has_chassis_contact_(false)
        , body_contact_points_(1.5)
    {
		tuning_.m_maxSuspensionTravelCm = 500;

		btTransform  tr;
		tr.setIdentity();
		btVector3 aabbMin,aabbMax;
#if 0
        chassis_shape_.get()->getAabb(tr,aabbMin,aabbMax);
#else
        chassis_shape_->getAabb(tr,aabbMin,aabbMax);
#endif

		btScalar dxx = btScalar(params_.wingspan / 2);
		btScalar dyy = btScalar(params_.length / 2);
		btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
		btScalar m12 = btScalar((params_.mass) /12);
		btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

        btDefaultMotionState* motionState = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(params_.mass), NULL/*new custom_ms*//*motionState*/, chassis_shape_/*&*chassis_shape_.get()*/, inertia);
		chassis_.reset(phys::bt_rigid_body_ptr(boost::make_shared<btRigidBody>(chassis_construction_info)));

		// FIXME TODO
		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
FIXME("Off point for bullet")
#if 1
		chassis_->setLinearVelocity(to_bullet_vector3(pos.dpos));
#endif
		//chassis_->setDamping(0.05f, 0.5f);
		chassis_->setRestitution(0.1f);
		//chassis_->setActivationState(DISABLE_DEACTIVATION);
		chassis_->setFriction(0.3f);

		raycast_veh_.reset(boost::make_shared<btRaycastVehicle>(tuning_, &*chassis_.get(), &*sys->vehicle_raycaster()));

		raycast_veh_->setCoordinateSystem(0,2,1);


		sys_->dynamics_world()->addAction(this);

		sys_->register_rigid_body(this);

		chassis_->setUserPointer(this);
    }

	void impl::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
	{

        if (control_manager_)
            control_manager_(deltaTimeStep);
        update_aerodynamics(deltaTimeStep);
	}

    void impl::update_aerodynamics(double dt)
    {
        using namespace cg;

        if (!chassis_->isActive())
            return;

        // special aircraft forces
            point_3 vel = from_bullet_vector3(chassis_->getLinearVelocity());
        double speed = cg::norm(vel - wind_);

            quaternion orien = from_bullet_quaternion(chassis_->getOrientation());
            point_3 omega = cg::rad2grad() * from_bullet_vector3(chassis_->getAngularVelocity());
            point_3 omega_loc = (!orien).rotate_vector(omega);
        dcpr cur_dcpr = cg::rot_axis2dcpr(orien.cpr(), rot_axis(omega));

        double const air_density = 1.225;
//            double const g = 9.8;
        double const q = 0.5 * air_density * speed * speed; // dynamic pressure

            point_3 forward_dir = cg::normalized_safe(orien.rotate_vector(point_3(0, 1, 0))) ;
            point_3 right_dir   = cg::normalized_safe(orien.rotate_vector(point_3(1, 0, 0))) ;
            point_3 up_dir      = cg::normalized_safe(orien.rotate_vector(point_3(0, 0, 1))) ;

            point_3 vk = vel - wind_;
            point_3 Y = !cg::eq_zero(cg::norm(vk)) ? cg::normalized(vk) : forward_dir;

            point_3 Z = cg::normalized_safe(right_dir ^ Y);
            point_3 X = cg::normalized_safe(Y ^ Z);

            point_3 Y_right_dir_proj =  Y - Y * right_dir * right_dir;
        double attack_angle = cg::rad2grad(cg::angle(Y_right_dir_proj, forward_dir)) * (-cg::sign(Y * up_dir));
        double slide_angle = cg::rad2grad(cg::angle(Y, Y_right_dir_proj))  * (-cg::sign(Y * right_dir));


        // drag - lift - slide
        double drag = 0;
        double lift = 0;
        double liftAOA = 0;
        double slide = 0;
        if (speed > params_.min_aerodynamic_speed)
        {
            double Cd = params_.Cd0 + params_.Cd2 * cg::sqr(params_.Cl);

            drag = Cd * params_.S * q;
            lift = params_.Cl * params_.S * q;
            liftAOA = params_.ClAOA * (attack_angle + params_.aa0) * params_.S * q;
            slide = params_.Cs * slide_angle * params_.S * q;
        }   

        double M_roll = 0;
        double M_course = 0;
        double M_pitch = 0;

        if (speed > params_.min_aerodynamic_speed)
        {
            M_roll += -params_.roll_sliding * slide_angle * q * params_.wingspan * params_.S;
            M_roll += -params_.roll_omega_y * omega_loc.y * q * params_.wingspan * params_.S;
            M_roll += params_.roll_omega_z * omega_loc.z * q * params_.wingspan * params_.S;

            M_roll += params_.ailerons * ailerons_ * q * params_.wingspan * params_.S;

            M_course += params_.course_sliding * slide_angle * q * params_.wingspan * params_.S;
            M_course += -params_.course_omega_z * omega_loc.z * q * params_.wingspan * params_.S;
            M_course += params_.course_omega_y * omega_loc.y * q * params_.wingspan * params_.S;
            M_course += params_.rudder * rudder_ * q * params_.wingspan * params_.S;

            M_pitch +=  params_.pitch_drag * q * params_.chord * params_.S;
            M_pitch +=  params_.pitch_attack * attack_angle * q * params_.chord * params_.S;
            M_pitch += -params_.pitch_omega_x * omega_loc.x * q * params_.chord * params_.S;
            M_pitch += -params_.pitch_attack_derivative * (attack_angle - prev_attack_angle_) / dt * q * params_.chord * params_.S;

            M_pitch += params_.elevator * elevator_ * q * params_.chord * params_.S;
        }


//            double zmoment = -Izz() * cg::rad2grad() * params_.Cl * params_.S * 0.5 * air_density * speed * sin(cg::grad2rad(orien.get_roll())) / params_.mass;

            point_3 torq = M_pitch * right_dir + M_roll * forward_dir + M_course * up_dir;
        //chassis_->applyTorque(to_bullet_vector3(torq*cg::grad2rad()));
FIXME("Off point for bullet")
#if 1
        chassis_->applyTorqueImpulse(to_bullet_vector3(torq*(cg::grad2rad()*dt)));
#endif

            point_3 force = ((lift + liftAOA) * Z - drag * Y + slide * X + params_.thrust * thrust_ * forward_dir);

        //LogTrace("lift " << lift << " liftAOA " << liftAOA << " drag " << drag << " thrust_ " << thrust_ );

        //chassis_->applyCentralForce(to_bullet_vector3(force));
FIXME("Off point for bullet")
#if 1
        chassis_->applyCentralImpulse(to_bullet_vector3(force * dt));
#endif

        prev_attack_angle_ = attack_angle;

        //std::stringstream cstr;

        //cstr << "Linear velocity : " << get_body()->getLinearVelocity().norm()  <<"\n" ;

        //OutputDebugString(cstr.str().c_str());

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
#else
    void impl::debugDraw(btIDebugDraw* debugDrawer)
    {
        if (raycast_veh_->getNumWheels() == 0)
            return;

        raycast_veh_->debugDraw(debugDrawer);
    }
#endif

    bool impl::has_contact() const
    {
        for (int i=0;i<raycast_veh_->getNumWheels();i++)
        {
            btWheelInfo const& info = raycast_veh_->getWheelInfo(i);
            if (info.m_raycastInfo.m_isInContact)
                return true;
        }

        if (has_chassis_contact_)
            return true;

        return false;
    }

//#define SIMEX_MOD
	size_t impl::add_wheel( double /*mass*/, double /*width*/, double radius, point_3 const& offset, cpr const & /*orien*/, bool /*has_damper*/, bool is_front )
	{
		point_3 connection_point = offset;
		// Source
        btScalar suspensionRestLength = 0.0f;
#if defined(SIMEX_MOD)
        connection_point.z += 1;
        suspensionRestLength = 1.0f;
#endif

        // Source
        // btWheelInfo& info = raycast_veh_->addWheel(to_bullet_vector3(connection_point),btVector3(0,0,-1),btVector3(1,0,0), 1.0f,btScalar(radius),tuning_,is_front);
        //tuning_.m_suspensionStiffness = 5.f; // 20.f
        //tuning_.m_suspensionDamping = 2.3f;
        FIXME(Ќе сделать ли массо-завимимым параметром) 
//#if !defined(SIMEX_MOD)
        tuning_.m_maxSuspensionForce = 60000000;
//#endif
        //tuning_.m_suspensionCompression = 4.4f;
        //tuning_.m_frictionSlip = 10.;

		btWheelInfo& info = raycast_veh_->addWheel(phys::to_bullet_vector3(connection_point),btVector3(0,0,-1),btVector3(1,0,0), suspensionRestLength,btScalar(radius),tuning_,is_front);
        info.m_suspensionStiffness = 20.f; 
		info.m_wheelsDampingRelaxation = 2.3f;
		info.m_wheelsDampingCompression = 4.4f;
        info.m_frictionSlip = 10.;
		info.m_rollInfluence = 0.1f;

        return wheels_ids_.insert(raycast_veh_->getNumWheels()-1);
	}
    
    void impl::remove_wheel(size_t id)
    {
        size_t num = wheels_ids_[id];
        size_t end_num = raycast_veh_->m_wheelInfo.size()-1;
        auto it = std::find(wheels_ids_.begin(), wheels_ids_.end(), end_num);
        std::swap(wheels_ids_[id], *it);
        raycast_veh_->m_wheelInfo.swap(num, end_num);
        raycast_veh_->m_wheelInfo.pop_back();
        wheels_ids_.erase(id);
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
    
    double impl::get_steer   ()
    {
        return steer_;
    }

	void impl::set_brake   (double brake)
	{
		for (int i=0;i<raycast_veh_->getNumWheels();i++)
		{
			raycast_veh_->setBrake(btScalar(brake * 10),i);
		}
	}

    void impl::set_thrust  (double thrust)
    {
        thrust_ = thrust;
        if (!cg::eq_zero(thrust))
            chassis_->activate();
    }

    void impl::apply_force (point_3 const& f)
    {
        chassis_->applyCentralForce(to_bullet_vector3(f));
    }

    void impl::set_elevator(double elevator)
    {
        elevator_ = elevator;
    }

    void impl::set_ailerons(double ailerons)
    {
        ailerons_ = ailerons;
    }

    void impl::set_rudder  (double rudder)
    {
        rudder_ = rudder;
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

		raycast_veh_.activate(chassis_->isActive());
	}

    void impl::has_contact(rigid_body_user_info_t const* /*other*/, point_3 const& local_point, point_3 const& vel)
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

    bool impl::has_wheel_contact(size_t id) const
    {
        btWheelInfo const& info = raycast_veh_->getWheelInfo(wheels_ids_[id]);
        return info.m_raycastInfo.m_isInContact;
    }

    double impl::wheel_skid_info(size_t id) const
    {
        btWheelInfo const& info = raycast_veh_->getWheelInfo(wheels_ids_[id]);
        return info.m_skidInfo;
    }

    void impl::reset_suspension()
    {
        for (int i=0;i<raycast_veh_->getNumWheels();i++)
        {
            raycast_veh_->resetSuspension();
            raycast_veh_->updateWheelTransform(i,false);
//            raycast_veh_->applyEngineForce(0,i);
        }
        
    }

    decart_position impl::get_position() const
    {
        return from_bullet_position(&*chassis_.get());
    }
	
	void impl::set_position(const decart_position& pos)
	{
		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
	}

    decart_position impl::get_wheel_position( size_t id ) const
    {
        auto tr  = raycast_veh_->getWheelInfo(wheels_ids_[id]).m_worldTransform;
        decart_position pos;
        pos.pos = from_bullet_vector3(tr.getOrigin());
        pos.orien = from_bullet_quaternion(tr.getRotation());

        return pos;
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

    double impl::thrust() const
    {
        return thrust_;
    }

    double impl::drag() const
    {
        cg::point_3 vel = from_bullet_vector3(chassis_->getLinearVelocity());
        double speed = cg::norm(vel);

        double const air_density = 1.225;
        double const q = 0.5 * air_density * speed * speed; // dynamic pressure

        double Cd = params_.Cd0 + params_.Cd2 * cg::sqr(params_.Cl);

        double drag = 0;
        if (speed > 5)
            drag = Cd * params_.S * q;
                                                    
        return drag;
    }

    double impl::lift() const
    {
        cg::point_3 vel = from_bullet_vector3(chassis_->getLinearVelocity());
        double speed = cg::norm(vel);

        double const air_density = 1.225;
        double const q = 0.5 * air_density * speed * speed; // dynamic pressure

        double lift = 0;
        if (speed > 5)
        {
            lift = params_.Cl * params_.S * q;
        }   

        return lift;
    }


}

}
