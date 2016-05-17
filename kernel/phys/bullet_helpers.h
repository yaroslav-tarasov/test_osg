#pragma once

// #include "phys_sys/phys_sys.h"
// #include "atc/position.h"
#include "../cpp_utils/polymorph_ptr.h"

class btRigidBody;
class btSoftBody;
class btCollisionShape;
class btDynamicsWorld;
class btTypedConstraint;
class btPoint2PointConstraint;
class btGeneric6DofConstraint;
class btCompoundShape;
class btDefaultVehicleRaycaster;
class btRaycastVehicle;

namespace phys
{
    typedef polymorph_ptr<btRigidBody>               bt_rigid_body_ptr;
    typedef polymorph_ptr<btCollisionShape>          bt_collision_shape_ptr;
    typedef polymorph_ptr<btDynamicsWorld>           bt_dynamics_world_ptr;
    typedef polymorph_ptr<btTypedConstraint>         bt_constraint_ptr;
    typedef polymorph_ptr<btPoint2PointConstraint>   bt_point_point_constraint_ptr;
    typedef polymorph_ptr<btGeneric6DofConstraint>   bt_generic_constraint_ptr;
    typedef polymorph_ptr<btCompoundShape>           bt_compound_shape_ptr;
    typedef polymorph_ptr<btDefaultVehicleRaycaster> bt_vehicle_raycaster_ptr;
    typedef polymorph_ptr<btRaycastVehicle>          bt_raycast_vehicle_ptr;
    typedef polymorph_ptr<btSoftBody>                bt_soft_body_ptr;

    inline btVector3 to_bullet_vector3( cg::point_3 const& v )
    {
        return btVector3(btScalar(v.x), btScalar(v.y), btScalar(v.z));
    }

    inline cg::point_3 from_bullet_vector3( btVector3 const& v )
    {
        return cg::point_3(v.x(), v.y(), v.z());
    }

    inline btMatrix3x3 to_bullet_matrix( cg::matrix_3 mm )
    {                 
        btMatrix3x3 m;
        m.setValue(btScalar(mm(0, 0)), btScalar(mm(0, 1)), btScalar(mm(0, 2)), 
            btScalar(mm(1, 0)), btScalar(mm(1, 1)), btScalar(mm(1, 2)),
            btScalar(mm(2, 0)), btScalar(mm(2, 1)), btScalar(mm(2, 2)));
        return m;
    }

    inline cg::matrix_3 from_bullet_matrix( btMatrix3x3 const& m )
    {
        cg::matrix_3 mm;
        mm(0,0) = m.getRow(0).x(), mm(0,1) = m.getRow(0).y(), mm(0,2) = m.getRow(0).z();
        mm(1,0) = m.getRow(1).x(), mm(1,1) = m.getRow(1).y(), mm(1,2) = m.getRow(1).z();
        mm(2,0) = m.getRow(2).x(), mm(2,1) = m.getRow(2).y(), mm(2,2) = m.getRow(2).z();

        return mm;
    }

    inline cg::quaternion from_bullet_quaternion( btQuaternion const& q )
    {                    
        return cg::quaternion(q.w(), cg::point_3(q.x(), q.y(), q.z()));
    }

    inline btQuaternion to_bullet_quaternion( cg::quaternion const& q )
    {                    
        return btQuaternion((btScalar)q.get_v().x, (btScalar)q.get_v().y, (btScalar)q.get_v().z, (btScalar)q.get_w());
    }

    inline btTransform to_bullet_transform( cg::transform_4 const& tr )
    {
        return btTransform(to_bullet_matrix(tr.rotation().matrix()), to_bullet_vector3(tr.translation()));
    }

    inline cg::transform_4 from_bullet_transform( btTransform const& tr )
    {
        return cg::transform_4(cg::as_translation(from_bullet_vector3(tr.getOrigin())), cg::rotation_3(from_bullet_matrix(tr.getBasis())));
    }

    inline btTransform to_bullet_transform( cg::point_3 const& pos, cg::cpr const& orien )
    {
        return btTransform(to_bullet_matrix(cg::rotation_3(orien).matrix()), to_bullet_vector3(pos));
    }

    inline btTransform to_bullet_transform( cg::point_3 const& pos, cg::quaternion const& orien )
    {
        return btTransform(to_bullet_quaternion(orien), to_bullet_vector3(pos));
    }

#ifndef BULLET_FROM_TO_ONLY

    inline decart_position from_bullet_position(btRigidBody const* body)
    {
        decart_position pos;

        btTransform tr = body->getCenterOfMassTransform();
        pos.pos   = from_bullet_vector3(tr.getOrigin());
        pos.orien = cg::rotation_3(from_bullet_matrix(tr.getBasis())).cpr();
        btVector3 vel = body->getLinearVelocity();
        pos.dpos = from_bullet_vector3(vel.length() < 1e-1?btVector3(0,0,0):vel);
        btVector3 avel = body->getAngularVelocity();
        pos.omega = cg::rad2grad() * from_bullet_vector3(avel.length() < 1e-1?btVector3(0,0,0):avel);

        return pos;
    }

    struct compound_shape_proxy
    {
        compound_shape_proxy()
            : shape_(new btCompoundShape())
        {

        }

		explicit compound_shape_proxy(btCompoundShape * cs)
			: shape_(cs)
		{

		}


        void add(bt_collision_shape_ptr shape, cg::transform_4 const& tr)
        {
            shape_->addChildShape(to_bullet_transform(tr), &*shape);
            childs_.push_back(shape);
        }

        bt_compound_shape_ptr get()
        {
            return shape_;
        }

        void reset (btCompoundShape * cs = nullptr)
        {
             shape_.reset(cs);
        }

		btCompoundShape * operator->() { return shape_.get();}
		btCompoundShape const* operator->() const { return shape_.get();}
    
	private:
       bt_compound_shape_ptr shape_;
       std::vector<bt_collision_shape_ptr> childs_;
    };

    template<class constraint_ptr>
    struct constraint_proxy
    {
        constraint_proxy(bt_dynamics_world_ptr dynamics_world, constraint_ptr constraint = constraint_ptr())
            : dynamics_world_(dynamics_world)
            , constraint_    (constraint)
        {
            if (constraint_)
                dynamics_world_->addConstraint(&*constraint_, true);
        }

        ~constraint_proxy()
        {
            if (constraint_)
                dynamics_world_->removeConstraint(&*constraint_);
        }

        void reset(constraint_ptr constraint = constraint_ptr())
        {
            if (constraint_)
                dynamics_world_->removeConstraint(&*constraint_);

            constraint_ = constraint;
            if (constraint_)
                dynamics_world_->addConstraint(&*constraint_, true);
        }

        constraint_ptr get() const
        {
            return constraint_;
        }

    private:
        bt_dynamics_world_ptr dynamics_world_;
        constraint_ptr constraint_;
    };


    struct rigid_body_proxy
    {
        rigid_body_proxy(bt_dynamics_world_ptr dynamics_world, bt_rigid_body_ptr body = bt_rigid_body_ptr())
            : dynamics_world_(dynamics_world)
            , body_          (body)
        {
            if (body_)
                dynamics_world_->addRigidBody(&*body_);
        }

        ~rigid_body_proxy()
        {
            if (body_)
                dynamics_world_->removeRigidBody(&*body_);
        }

        void reset(bt_rigid_body_ptr body = bt_rigid_body_ptr())
        {
            if (body_)
                dynamics_world_->removeRigidBody(&*body_);

            body_ = body;
            if (body_)
                dynamics_world_->addRigidBody(&*body_);
        }

        operator bool() const
        {
            return body_;
        }

        bt_rigid_body_ptr get() const
        {
            return body_;
        }

        btRigidBody * operator->() { return body_.get();}
        btRigidBody const* operator->() const { return body_.get();}


    private:
        bt_dynamics_world_ptr dynamics_world_;
        bt_rigid_body_ptr body_;
    };


    struct raycast_vehicle_proxy
    {
        raycast_vehicle_proxy(bt_dynamics_world_ptr dynamics_world)
            : dynamics_world_(dynamics_world)
            , active_(false)
        {
        }

        ~raycast_vehicle_proxy()
        {
            if (self_ && active_)
                dynamics_world_->removeVehicle(&*self_);
        }

        void reset(bt_raycast_vehicle_ptr self = bt_raycast_vehicle_ptr())
        {
            if (self_ && active_)
                dynamics_world_->removeVehicle(&*self_);

            self_ = self;
            active_ = false;
        }

        btRaycastVehicle * get() const
        {
            return self_.get();
        }

        void activate(bool active)
        {
            if (active_ != active)
            {
                //LogTrace("vehicle active " << active);

                if (active)
                    dynamics_world_->addVehicle(&*self_);
                else
                    dynamics_world_->removeVehicle(&*self_);
                active_ = active;
            }
        }


        bool is_active() const { return active_; }

        btRaycastVehicle * operator->() { return self_.get();}
        btRaycastVehicle const* operator->() const { return self_.get();}

    private:
        bt_dynamics_world_ptr        dynamics_world_;
        bt_raycast_vehicle_ptr       self_;

        bool active_;
    };

    struct	custom_ms : public btMotionState
    {
        void	getWorldTransform(btTransform& worldTrans ) const override
        {}

        //Bullet only calls the update of worldtransform for active objects
        void	setWorldTransform(const btTransform& worldTrans) override
        {
              int i = 0;
        }
    };
#endif

}