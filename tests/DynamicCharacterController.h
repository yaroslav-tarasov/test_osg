#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "LinearMath/btVector3.h"

#include "BulletDynamics/Character/btCharacterControllerInterface.h"

class btCollisionShape;
class btRigidBody;
class btCollisionWorld;

///DynamicCharacterController is obsolete/unsupported at the moment
class DynamicCharacterController : public btCharacterControllerInterface
{
protected:
	btScalar m_halfHeight;
	btCollisionShape* m_shape;
	btRigidBody* m_rigidBody;

	btVector3 m_raySource[2];
	btVector3 m_rayTarget[2];
	btScalar m_rayLambda[2];
	btVector3 m_rayNormal[2];

	btScalar m_turnAngle;

	btScalar m_maxLinearVelocity;
	btScalar m_walkVelocity;
	btScalar m_turnVelocity;
public:
	DynamicCharacterController ();
	~DynamicCharacterController ();
	void setup (btScalar height = 2.0, btScalar width = 0.25, btScalar stepHeight = 0.25);
	void destroy ();

	virtual void reset (btCollisionWorld *);
	virtual void warp (const btVector3& origin);
	virtual void registerPairCacheAndDispatcher (btOverlappingPairCache* pairCache, btCollisionDispatcher* dispatcher);
	virtual void	setUpInterpolate (bool value);

    virtual void	setWalkDirection(const btVector3& walkDirection) {};
    virtual void	setVelocityForTimeInterval(const btVector3& velocity, btScalar timeInterval) {};

    btCollisionObject* getCollisionObject ();

	virtual void preStep (btCollisionWorld* collisionWorld);
	void playerStep (btCollisionWorld* collisionWorld,btScalar dt/*,
					 int forward,
					 int backward,
					 int left,
					 int right,
					 int jump*/);
	bool canJump () const;
	void jump ();

	bool onGround () const;

    ///btActionInterface interface
    virtual void updateAction( btCollisionWorld* collisionWorld,btScalar deltaTime)
    {
        preStep ( collisionWorld);
        playerStep (collisionWorld, deltaTime);
    }
    ///btActionInterface interface
    void	debugDraw(btIDebugDraw* debugDrawer);
};

#endif
