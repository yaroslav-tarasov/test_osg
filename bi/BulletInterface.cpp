#include "stdafx.h"

#include "BulletInterface.h"



struct FilterCallback : public btOverlapFilterCallback
{
    // return true when pairs need collision
    virtual bool	needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
    {
        bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
        collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

        //add some additional logic here that modified 'collides'
        return collides;
    }
};                 





BulletInterface* BulletInterface::instance()
{
    static osg::ref_ptr<BulletInterface> s_registry = new BulletInterface;
    return s_registry.get();
}

BulletInterface::BulletInterface()
    : _scene(nullptr)
    , _on_collision(nullptr)
    , _dd (nullptr)
{

    _configuration = new btDefaultCollisionConfiguration;
    _dispatcher = new btCollisionDispatcher( _configuration );
    _overlappingPairCache = new btDbvtBroadphase;
    _solver = new btSequentialImpulseConstraintSolver;

}

BulletInterface::~BulletInterface()
{
    if ( _scene )
    {
        for ( int i=_scene->getNumCollisionObjects()-1; i>=0; --i )
        {
            btCollisionObject* obj = _scene->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if ( body && body->getMotionState() )
                delete body->getMotionState();
            
            _scene->removeCollisionObject( obj );
            delete obj;
        }
        delete _scene;
    }
    
    delete _solver;
    delete _overlappingPairCache;
    delete _dispatcher;
    delete _configuration;
}


void BulletInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity, on_collision_f on_collision )
{
    _scene = new btDiscreteDynamicsWorld( _dispatcher, _overlappingPairCache, _solver, _configuration );
    _scene->setGravity( btVector3(gravity[0], gravity[1], gravity[2]) );
    
    //static osgbDynamics::PhysicsThread pt( _scene, &tBuf );

    osg::Vec3 norm = plane.getNormal();
    btCollisionShape* groundShape = new btStaticPlaneShape( btVector3(norm[0], norm[1], norm[2]), plane[3] );
    btTransform groundTransform;
	groundTransform.setIdentity();
	
    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( 0.0, motionState, groundShape, btVector3(0.0, 0.0, 0.0) );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body );
    body->setFriction(1.3f); 
    body->setActivationState(DISABLE_SIMULATION);
    body->setRestitution(0.5f);

    _on_collision = on_collision;

}

void BulletInterface::createBox( int id, const osg::Vec3& dim, double density )
{
    btCollisionShape* boxShape = new btBoxShape( btVector3(dim[0], dim[1], dim[2]) );
    btTransform boxTransform;
	boxTransform.setIdentity();
	
	btVector3 localInertia(0.0, 0.0, 0.0);
	if ( density>0.0 )
	    boxShape->calculateLocalInertia( density, localInertia );
	
    btDefaultMotionState* motionState = new btDefaultMotionState(boxTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( density, motionState, boxShape, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body); 
    _actors[id]._body = body;
    _actors[id]._type  = BOX;
}

void BulletInterface::createSphere( int id, double radius, double density )
{
    btCollisionShape* sphereShape = new btSphereShape( radius );
    btTransform sphereTransform;
	sphereTransform.setIdentity();
	
	btVector3 localInertia(0.0, 0.0, 0.0);
	if ( density>0.0 )
	    sphereShape->calculateLocalInertia( density, localInertia );
	
    btDefaultMotionState* motionState = new btDefaultMotionState(sphereTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( density, motionState, sphereShape, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body );
    body->setFriction(10.3f);
    body->setRestitution(0.1f);
    _actors[id]._body  = body;
    _actors[id]._type  = SPHERE;
}

void BulletInterface::setVelocity( int id, const osg::Vec3& vec )
{
    btRigidBody* actor = _actors[id]._body;
    if ( actor )
        actor->setLinearVelocity( btVector3(vec.x(), vec.y(), vec.z()) );
}

void BulletInterface::setMatrix( int id, const osg::Matrix& matrix )
{
    btRigidBody* actor = _actors[id]._body;
    if ( actor )
    {
        btTransform trans;
        trans.setFromOpenGLMatrix( osg::Matrixf(matrix).ptr() );
        actor->setWorldTransform( trans );
    }
}

osg::Matrix BulletInterface::getMatrix( int id )
{
    btRigidBody* actor = _actors[id]._body;
    if ( actor )
    {
        btTransform trans = actor->getWorldTransform();
        osg::Matrixf matrix;
        trans.getOpenGLMatrix( matrix.ptr() );
        return matrix;
    }
    return osg::Matrix();
}

void BulletInterface::simulate( double step )
{
    _scene->stepSimulation( step, 10 );
    checkForCollisionEvents();
}


void BulletInterface::checkForCollisionEvents() {
	// keep a list of the collision pairs we
	// found during the current update
	CollisionPairs pairsThisUpdate;
	
	// iterate through all of the manifolds in the dispatcher
	for (int i = 0; i < _dispatcher->getNumManifolds(); ++i) {
			
		// get the manifold
		btPersistentManifold* pManifold = _dispatcher->getManifoldByIndexInternal(i);
			
		// ignore manifolds that have 
		// no contact points.
		if (pManifold->getNumContacts() > 0) {
			// get the two rigid bodies involved in the collision
			const btRigidBody* pBody0 = static_cast<const btRigidBody*>(pManifold->getBody0());
			const btRigidBody* pBody1 = static_cast<const btRigidBody*>(pManifold->getBody1());
			// always create the pair in a predictable order
			// (use the pointer value..)
			bool const swapped = pBody0 > pBody1;
			const btRigidBody* pSortedBodyA = swapped ? pBody1 : pBody0;
			const btRigidBody* pSortedBodyB = swapped ? pBody0 : pBody1;
				
			// create the pair
			CollisionPair thisPair = std::make_pair(pSortedBodyA, pSortedBodyB);
				
			// insert the pair into the current list
			pairsThisUpdate.insert(thisPair);
	
			// if this pair doesn't exist in the list
			// from the previous update, it is a new
			// pair and we must send a collision event
			if (_pairsLastUpdate.find(thisPair) == _pairsLastUpdate.end()) {
				CollisionEvent((btRigidBody*)pBody0, (btRigidBody*)pBody1);
			}
		}
	}
		
	//// create another list for pairs that
	//// were removed this update
	//CollisionPairs removedPairs;
	//	
	//// this handy function gets the difference beween
	//// two sets. It takes the difference between
	//// collision pairs from the last update, and this 
	//// update and pushes them into the removed pairs list
	//std::set_difference( m_pairsLastUpdate.begin(), m_pairsLastUpdate.end(),
	//pairsThisUpdate.begin(), pairsThisUpdate.end(),
	//std::inserter(removedPairs, removedPairs.begin()));
	//	
	//// iterate through all of the removed pairs
	//// sending separation events for them
	//for (CollisionPairs::const_iterator iter = removedPairs.begin(); iter != removedPairs.end(); ++iter) {
	//	SeparationEvent((btRigidBody*)iter->first, (btRigidBody*)iter->second);
	//}
	//	
	//// in the next iteration we'll want to
	//// compare against the pairs we found
	//// in this iteration
	_pairsLastUpdate = pairsThisUpdate;
}


void BulletInterface::CollisionEvent(btRigidBody * pBody0, btRigidBody * pBody1) {
    // find the two colliding objects
    
    for (auto it = _actors.begin();it!=_actors.end();++it)
    {
        if(pBody0 == it->second._body && it->second._type == SPHERE)
        {
            if (_on_collision)
                 _on_collision(it->first);

        }

        if(pBody1 == it->second._body)
        {

        }

    }
    //GameObject* pObj0 = FindGameObject(pBody0);
    //GameObject* pObj1 = FindGameObject(pBody1);

    //// exit if we didn't find anything
    //if (!pObj0 || !pObj1) return;

    //// set their colors to white
    //pObj0->SetColor(btVector3(1.0,1.0,1.0));
    //pObj1->SetColor(btVector3(1.0,1.0,1.0));
}