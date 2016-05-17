#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include "bullet_helpers.h"

#include "flock_child_phys.h"


namespace phys
{
	namespace flock
	{

    impl::impl(system_impl_ptr sys,compound_sensor_ptr s,/*compound_shape_proxy& s,*/ params_t const& params, decart_position const& pos)
        : bt_body_user_info_t(rb_flock_child)
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


		btScalar dxx = btScalar((aabbMax.x() - aabbMin.x()) / 2);// btScalar(params_.wingspan / 2);
		btScalar dyy = btScalar((aabbMax.y() - aabbMin.y()) / 2);// btScalar(params_.length / 2);
		btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
		btScalar m12 = btScalar((params_.mass) /12);
		btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

        btDefaultMotionState* motionState = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(params_.mass), /*NULL*//*new custom_ms*/motionState, chassis_shape_/*&*chassis_shape_.get()*/, inertia);
		chassis_.reset(phys::bt_rigid_body_ptr(boost::make_shared<btRigidBody>(chassis_construction_info)));

		// FIXME TODO
		chassis_->setCenterOfMassTransform(to_bullet_transform(pos.pos, pos.orien.cpr()));
#if 0
		chassis_->setLinearVelocity(to_bullet_vector3(pos.dpos));
#endif
		//chassis_->setDamping(0.05f, 0.5f);
		chassis_->setRestitution(0.1f);
		//chassis_->setActivationState(DISABLE_DEACTIVATION);
		chassis_->setFriction(0.3f);

		// sys_->dynamics_world()->addAction(this);

		sys_->register_rigid_body(this);

		chassis_->setUserPointer(this);
    }
    
    impl::~impl()
    {
        chassis_.reset();
        sys_->unregister_rigid_body(this);
        // sys_->dynamics_world()->removeAction(this);
    }

	void impl::updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep)
	{
        update_aerodynamics(deltaTimeStep);
	}

    void impl::update_aerodynamics(double dt)
    {
        using namespace cg;

        if (!chassis_->isActive())
            return;
#if 0

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

        point_3 torq = M_pitch * right_dir + M_roll * forward_dir + M_course * up_dir;
        //chassis_->applyTorque(to_bullet_vector3(torq*cg::grad2rad()));
        chassis_->applyTorqueImpulse(to_bullet_vector3(torq*(cg::grad2rad()*dt)));


        point_3 force = ((lift + liftAOA) * Z - drag * Y + slide * X + params_.thrust * thrust_ * forward_dir);

        //LogTrace("lift " << lift << " liftAOA " << liftAOA << " drag " << drag << " thrust_ " << thrust_ );

        //chassis_->applyCentralForce(to_bullet_vector3(force));
        chassis_->applyCentralImpulse(to_bullet_vector3(force * dt));

        prev_attack_angle_ = attack_angle;
#endif
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
