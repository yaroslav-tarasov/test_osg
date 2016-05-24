#include "stdafx.h"

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
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


namespace {

    inline bt_soft_body_ptr	create_rope(	btSoftBodyWorldInfo& worldInfo, const btVector3& from,
        const btVector3& to,
        int res,
        int fixeds)
    {
        /* Create nodes	*/ 
        const int		r=res+2;
        btVector3*		x=new btVector3[r];
        btScalar*		m=new btScalar[r];
        int i;

        for(i=0;i<r;++i)
        {
            const btScalar	t=i/(btScalar)(r-1);
            x[i]=lerp(from,to,t);
            m[i]=1;
        }

        bt_soft_body_ptr		psb= boost::make_shared<btSoftBody>(&worldInfo,r,x,m);
        if(fixeds&1) psb->setMass(0,0);
        if(fixeds&2) psb->setMass(r-1,0);
        delete[] x;
        delete[] m;
        /* Create links	*/ 
        for(i=1;i<r;++i)
        {
            psb->appendLink(i-1,i);
        }
        /* Finished		*/ 
        return(psb);
    }

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
}



	namespace arresting_gear
	{

    impl::impl(system_impl_ptr sys,compound_sensor_ptr s,params_t const& params, decart_position const& pos)
        : bt_body_user_info_t(bt_soft_body)
		, sys_                  (sys)
#if 0
		, chassis_              (sys->dynamics_world())
        , chassis_shape_        (compound_sensor_impl_ptr(s)->cs_)
#endif
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

		btScalar dxx = btScalar((aabbMax.x() - aabbMin.x()) / 2);
		btScalar dyy = btScalar((aabbMax.y() - aabbMin.y()) / 2);
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
        bt_softrigid_dynamics_world_ptr bw = bt_softrigid_dynamics_world_ptr(sys->dynamics_world());

        const unsigned n = params.ropes.size();
        auto const & propes_params = params.ropes;
        ropes_.reserve(n);
        for( unsigned i=0; i<n; ++i)
        {
            ropes_.push_back(std::move(std::unique_ptr<soft_body_proxy>(new soft_body_proxy(bw))));

            ropes_.back().get()->reset(create_rope(bw->getWorldInfo(),	
                to_bullet_vector3(propes_params[i].first),
                to_bullet_vector3(propes_params[i].second),
                params.seg_num,
                1+2));

            auto& psb = *ropes_.back().get();
            psb->m_cfg.piterations		=	4;
            psb->m_materials[0]->m_kLST	=	0.1+(i/(btScalar)(n-1))*0.9;
            psb->setTotalMass(20);


#if 0
            if(i==0)
            {
                btTransform startTransform;
                startTransform.setIdentity();
                startTransform.setOrigin(btVector3(60 + i*step,30,0));
                btRigidBody*		body= create_rigid_body(bw.get(),50,startTransform,new btBoxShape(btVector3(2,6,2)));
                psb->appendAnchor(psb->m_nodes.size()/2,body);
                body->setLinearVelocity( btVector3(30.0,0.0,0.0) );
            }
#endif

        }


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

    std::vector<::arresting_gear::rope_state_t>   impl::get_ropes_info() const
    {
        std::vector<::arresting_gear::rope_state_t> res;
        res.reserve(ropes_.size());
        unsigned i = 0;
        for (auto it = ropes_.begin(); it != ropes_.end(); ++it, ++i)
        {
            const btSoftBody::tNodeArray& nodes = it->get()->get()->m_nodes;
            unsigned idx;
            ::arresting_gear::rope_state_t ri;
            ri.resize(nodes.size());
            for( idx=0; idx<nodes.size(); idx++)
            {
                ri[idx].coord  = from_bullet_vector3(nodes[ idx ].m_x);
                ri[idx].vel = from_bullet_vector3(nodes[ idx ].m_v);
                FIXME(Some kind of on may be placed here)
            }
            res.push_back(std::move(ri));
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
    
    void   impl::append_anchor        (rigid_body_ptr body, cg::point_3 const& pos)
    {
		auto& psb = *ropes_.back().get();
		psb->appendAnchor(psb->m_nodes.size()/2, rigid_body_impl_ptr(body)->get_body().get());
    }

    void   impl::release_anchor       (rigid_body_ptr body)
    {

    }
    
    void impl::set_target(rigid_body_ptr rb, cg::point_3 const& self_offset, cg::point_3 const& offset)
    {
         append_anchor        (rb, self_offset);
    }

    void impl::reset_target()
    {

    }

    params_t const& impl::params() const
    {
        return params_;
    }

}

}
