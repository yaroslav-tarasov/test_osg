#pragma once

#include <btBulletDynamicsCommon.h> 
#include "bullet_helpers.h"

namespace phys
{
    enum rigid_body_kind_t
    {
        rb_simple,
        rb_fracture,
        rb_terrain,
        rb_vehicle,
        rb_wheel,
        rb_plant,
        rb_static_collision,
        rb_character, 
        rb_aircraft,
        rb_static_convex,
        rb_flock_child
    };

    struct rigid_body_user_info_t // Must be first in derives, because static cast used in user_info
    {
        rigid_body_user_info_t(rigid_body_kind_t kind, bool has_collision = true)
            : kind_(kind)
            , has_collision_(has_collision)
        {}

        virtual ~rigid_body_user_info_t() {}

        rigid_body_kind_t rigid_body_kind() const
        {
            return kind_;
        }

        void disable_collision( bool disable )
        {
            has_collision_ = !disable;
        }

        bool has_collision() const { return has_collision_; }

        virtual float get_friction( const btCollisionObject* /*other*/ ) const { return 0.f; }
        virtual bool  has_collision_with( const btCollisionObject* /*other*/ ) const { return true; }

    private:
        bool has_collision_;
        rigid_body_kind_t kind_;
    };

    struct overlap_filter_callback : public btOverlapFilterCallback
    {
        // return true when pairs need collision
        virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
        {
            bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
            collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

            btCollisionObject * col0 = static_cast<btCollisionObject *>(proxy0->m_clientObject);
            btCollisionObject * col1 = static_cast<btCollisionObject *>(proxy1->m_clientObject);
            if (col0->getUserPointer() && col1->getUserPointer())
            {
                rigid_body_user_info_t * info0 = static_cast<rigid_body_user_info_t *>(col0->getUserPointer());
                rigid_body_user_info_t * info1 = static_cast<rigid_body_user_info_t *>(col1->getUserPointer());

                if (!info0->has_collision() || !info1->has_collision())
                    collides = false;
                else if (info0->rigid_body_kind() == rb_plant && info1->rigid_body_kind() == rb_plant)
                    collides = false;

                if (info0->rigid_body_kind() == rb_static_collision)
                    if (!info0->has_collision_with(col1))
                        collides = false;

                if (info1->rigid_body_kind() == rb_static_collision)
                    if (!info1->has_collision_with(col0))
                        collides = false;
            }

            //add some additional logic here that modified 'collides'
            return collides;
        }
    };

    inline btScalar    calculateCombinedFriction(btScalar friction0, btScalar friction1)
    {
        const btScalar MAX_FRICTION  = btScalar(10.);
        return cg::bound(friction0 * friction1, -MAX_FRICTION, MAX_FRICTION);
    }

    inline btScalar    calculateCombinedRestitution(btScalar restitution0, btScalar restitution1)
    {
        return restitution0 * restitution1;
    }

    static bool material_combiner_callback(btManifoldPoint& cp,    const btCollisionObject* colObj0,int /*partId0*/,int /*index0*/,const btCollisionObject* colObj1,int /*partId1*/,int /*index1*/)
    {
        float friction0 = colObj0->getFriction();
        float friction1 = colObj1->getFriction();
        float restitution0 = colObj0->getRestitution();
        float restitution1 = colObj1->getRestitution();

        if (colObj0->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
        {
            if (colObj0->getInternalType() == btCollisionObject::CO_RIGID_BODY)
            {
                btRigidBody const* body0 = static_cast<btRigidBody const*>(colObj0);
                if (body0->getUserPointer())
                {
                    rigid_body_user_info_t * info0 = static_cast<rigid_body_user_info_t *>(body0->getUserPointer());
                    friction0 = info0->get_friction(colObj1);
                }
            }
        }
        if (colObj1->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
        {
            if (colObj1->getInternalType() == btCollisionObject::CO_RIGID_BODY)
            {
                btRigidBody const* body1 = static_cast<btRigidBody const*>(colObj1);
                if (body1->getUserPointer())
                {
                    rigid_body_user_info_t * info1 = static_cast<rigid_body_user_info_t *>(body1->getUserPointer());
                    friction1 = info1->get_friction(colObj0);
                }
            }
        }

        cp.m_combinedFriction = calculateCombinedFriction(friction0,friction1);
        cp.m_combinedRestitution = calculateCombinedRestitution(restitution0,restitution1);

        //this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
        return true;
    }

}