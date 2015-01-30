#pragma once

// #include "phys_sys/phys_sys.h"
// #include "atc/position.h"
#include "../cpp_utils/polymorph_ptr.h"

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

    namespace osg_helpers
    {
        inline btVector3 to_bullet_vector3( osg::Vec3 const& v )
        {
            return btVector3(btScalar(v.x()), btScalar(v.y()), btScalar(v.z()));
        }

        inline osg::Vec3 from_bullet_vector3( btVector3 const& v )
        {
            return osg::Vec3(v.x(), v.y(), v.z());
        }

        inline btMatrix3x3 to_bullet_matrix( osg::Matrix mm )
        {                 
            btMatrix3x3 m;
            m.setValue(btScalar(mm(0, 0)), btScalar(mm(0, 1)), btScalar(mm(0, 2)), 
                btScalar(mm(1, 0)), btScalar(mm(1, 1)), btScalar(mm(1, 2)),
                btScalar(mm(2, 0)), btScalar(mm(2, 1)), btScalar(mm(2, 2)));
            return m;
        }

        inline osg::Matrix from_bullet_matrix( btMatrix3x3 const& m )
        {
            osg::Matrix mm;
            mm(0,0) = m.getRow(0).x(), mm(0,1) = m.getRow(0).y(), mm(0,2) = m.getRow(0).z();
            mm(1,0) = m.getRow(1).x(), mm(1,1) = m.getRow(1).y(), mm(1,2) = m.getRow(1).z();
            mm(2,0) = m.getRow(2).x(), mm(2,1) = m.getRow(2).y(), mm(2,2) = m.getRow(2).z();

            return mm;
        }

        inline osg::Quat from_bullet_quaternion( btQuaternion const& q )
        {                    
            return osg::Quat(q.x(), q.y(), q.z(),q.w());
        }

        inline btQuaternion to_bullet_quaternion( osg::Quat const& q )
        {                    
            return btQuaternion((btScalar)q.x(), (btScalar)q.y(), (btScalar)q.z(), (btScalar)q.w());
        }

        inline btTransform to_bullet_transform( osg::Matrix const& tr )
        {
            return osgbCollision::asBtTransform(tr);
        }

	    inline osg::Matrix from_bullet_transform( btTransform const& tr )
	    {
	        return osgbCollision::asOsgMatrix(tr);
	    }
    }
    
 

    inline osg::Matrix asOsgMatrix( const btTransform& t )
    {
        btScalar ogl[ 16 ];
        t.getOpenGLMatrix( ogl );
        osg::Matrix m( ogl );
        return m;
    }

    inline btTransform asBtTransform( const osg::Matrix& m )
    {
        const osg::Matrix::value_type* oPtr = m.ptr();
        btScalar bPtr[ 16 ];
        int idx;
        for (idx=0; idx<16; idx++)
            bPtr[ idx ] = oPtr[ idx ];
        btTransform t;
        t.setFromOpenGLMatrix( bPtr );
        return t;
    }    

    // TODO FIXME хочется путей по прямее
    inline cg::transform_4 from_osg_transform( const osg::Matrix& m )
    {
        btTransform tr = asBtTransform( m );
        return cg::transform_4(cg::as_translation(from_bullet_vector3(tr.getOrigin())), cg::rotation_3(from_bullet_matrix(tr.getBasis())));
    }

    //inline bt_collision_shape_ptr get_sensor_convex( sensor_ptr s )
    //{
    //    btAlignedObjectArray<btVector3> points;

    //    for (size_t i = 0; i < s->chunks_count(); ++i)
    //    {
    //        for (size_t j = 0; j < s->triangles_count(i); ++j)
    //        {
    //            cg::triangle_3f tr = s->triangle(i, j);
    //            for (size_t k = 0; k < 3; ++k) 
    //            {
    //                if (cg::norm(tr[k]) > 50)
    //                    continue;
    //                points.push_back(to_bullet_vector3(tr[k]));
    //            }
    //        }
    //    }

    //    return bt_collision_shape_ptr(new btConvexHullShape((btScalar *)&points[0], points.size()));
    //}

    //inline bt_collision_shape_ptr get_sensor_nonconvex( sensor const* s )
    //{
    //    // ONLY for test
    //    btTriangleMesh * mesh = new btTriangleMesh();

    //    for (size_t i = 0; i < s->chunks_count(); ++i)
    //    {
    //        for (size_t j = 0; j < s->triangles_count(i); ++j)
    //        {
    //            cg::triangle_3f tr = s->triangle(i, j);

    //            mesh->addTriangle(to_bullet_vector3(tr[0]), to_bullet_vector3(tr[1]), to_bullet_vector3(tr[2]), false);
    //        }
    //    }

    //    return bt_collision_shape_ptr(new btBvhTriangleMeshShape(mesh, true));
    //}

    inline decart_position from_bullet_position(btRigidBody const* body)
    {
        decart_position pos;

        btTransform tr = body->getCenterOfMassTransform();
        pos.pos   = from_bullet_vector3(tr.getOrigin());
        pos.orien = cg::rotation_3(from_bullet_matrix(tr.getBasis())).cpr();
        pos.dpos = from_bullet_vector3(body->getLinearVelocity());
        pos.omega = cg::rad2grad() * from_bullet_vector3(body->getAngularVelocity());

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

        void add(bt_collision_shape_ptr shape, osg::Matrix const& tr)
        {
            shape_->addChildShape(osg_helpers::to_bullet_transform(tr), &*shape);
            childs_.push_back(shape);
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

}