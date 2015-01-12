#include "stdafx.h"


#include "BulletInterface.h"
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>
#include "../find_node_visitor.h"
#include "../high_res_timer.h"
#include "../aircraft_phys.h"


using namespace phys;

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

static void internal_tick_callback(btDynamicsWorld *world, btScalar /*timeStep*/)
{

}

namespace phys
{
	BulletInterface*  sys()
	{
		static std::shared_ptr<BulletInterface> inst = create();
		return inst.get();
	}

	std::shared_ptr<BulletInterface> create()
	{
		return std::make_shared<BulletInterface>();
	}
}

inline osg::Matrix get_relative_transform(osg::Node* root, osg::Node* node, osg::Node* rel = NULL)
{
	osg::Matrix tr;
	osg::Node* n = node;
	while(/*n->position().is_local() &&*/ n != rel && 0 != n->getNumParents() && n != root)
	{
		tr = n->asTransform()->asMatrixTransform()->getMatrix() * tr;
		n = n->getParent(0);
	}

	if (n == rel || rel == NULL)
		return tr;

	osg::Matrix tr_rel;
	n = rel;
	while(/*n->position().is_local()*/0 != n->getNumParents() && n != root)
	{                  
		tr_rel = n->asTransform()->asMatrixTransform()->getMatrix() * tr_rel;
		n = n->getParent(0);
	}

	return (osg::Matrix::inverse(tr_rel)) * tr;
}

namespace 
{

	struct wheel_info
	{
		wheel_info(double radius,bool front)
			: radius(radius)
			, front(front)
		{}

		double      radius;
		osg::Matrix trans_f_body;
		bool        front;
	};

	typedef std::vector<wheel_info> wheels_info_t;

	btCompoundShape* fill_cs(osg::Node* node, wheels_info_t& wi,aircraft::params_t& p, double& shift )
    {          
        osg::ComputeBoundsVisitor cbv;
        node->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();
        
        shift = -zm/2;
        p.wingspan = xm;
        p.length   = ym;

        auto body   = findFirstNode(node,"Body",findNodeVisitor::not_exact);
		auto sh_r_l = findFirstNode(node,"animgroup_shassi_r_l",findNodeVisitor::not_exact);
		auto sh_r_r = findFirstNode(node,"animgroup_shassi_r_r",findNodeVisitor::not_exact);
		auto sh_f   = findFirstNode(node,"animgroup_shassi_f",findNodeVisitor::not_exact);

		auto sh_r_l_wheel = findFirstNode(sh_r_l,"wheel",findNodeVisitor::not_exact);
		auto sh_r_r_wheel = findFirstNode(sh_r_r,"wheel",findNodeVisitor::not_exact);
		auto sh_f_wheel   = findFirstNode(sh_f,"wheel",findNodeVisitor::not_exact);
		
		const bool is_front = true;
		{
			wheel_info wii(sh_f_wheel->getBound().radius(),is_front);
			wii.trans_f_body = get_relative_transform(node,sh_f_wheel/*,body*/);
			wi.push_back(wii);
		}
	    {
			wheel_info wii(sh_r_r_wheel->getBound().radius(),!is_front);
			wii.trans_f_body = get_relative_transform(node,sh_r_r_wheel/*,body*/);
			wi.push_back(wii);
		}
		{
			wheel_info wii(sh_r_l_wheel->getBound().radius(),!is_front);
			wii.trans_f_body = get_relative_transform(node,sh_r_l_wheel/*,body*/);
			wi.push_back(wii);
		}
		

		btTransform cmt(btQuaternion(0,0,0),btVector3(1,0,0));
		cmt.setIdentity();
		////localTrans effectively shifts the center of mass with respect to the chassis
		//cmt.setOrigin(btVector3(0,-10,0));	

		btCompoundShape*  s = new btCompoundShape;

		sh_f_wheel->setNodeMask(0);
		sh_r_r_wheel->setNodeMask(0);
		sh_r_l_wheel->setNodeMask(0);

		btCollisionShape* cs_sh_r_l = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( sh_r_l );
		btCollisionShape* cs_sh_r_r = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( sh_r_r );
		btCollisionShape* cs_sh_f   = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( sh_f );
		btCollisionShape* cs_body   = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( body );

		sh_f_wheel->setNodeMask(0xffffffff);
		sh_r_r_wheel->setNodeMask(0xffffffff);
		sh_r_l_wheel->setNodeMask(0xffffffff);

		s->addChildShape(btTransform(btQuaternion(0,0,0),btVector3(0,shift,0)),cs_sh_f);
		s->addChildShape(btTransform(btQuaternion(0,0,0),btVector3(0,shift,0)),cs_sh_r_l);
		s->addChildShape(btTransform(btQuaternion(0,0,0),btVector3(0,shift,0)),cs_sh_r_r);
		s->addChildShape(btTransform(btQuaternion(0,0,0),btVector3(0,shift,0)),cs_body);

		return s;
	}
}


#if 0
BulletInterface* BulletInterface::instance()
{
    static std::shared_ptr<BulletInterface> s_registry = std::make_shared<BulletInterface>();
    return s_registry.get();
}
#endif

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
        // delete _scene;
		_scene.reset();
    }
    
    delete _solver;
    delete _overlappingPairCache;
    delete _dispatcher;
    delete _configuration;
}


void BulletInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity, on_collision_f on_collision )
{
    _scene = std::make_shared<btDiscreteDynamicsWorld>(_dispatcher,_overlappingPairCache, _solver, _configuration);
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
	
	_scene->setInternalTickCallback(internal_tick_callback);

	vehicle_raycaster_.reset(new btDefaultVehicleRaycaster(&*_scene));

}

void BulletInterface::createBox( int id, const osg::Vec3& dim, double mass )
{
    btCollisionShape* boxShape = new btBoxShape( btVector3(dim[0], dim[1], dim[2]) );
    btTransform boxTransform;
	boxTransform.setIdentity();
	
	btVector3 localInertia(0.0, 0.0, 0.0);
	if ( mass>0.0 )
	    boxShape->calculateLocalInertia( mass, localInertia );
	
    btDefaultMotionState* motionState = new btDefaultMotionState(boxTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( mass, motionState, boxShape, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body); 
    _actors[id]._body = body;
    _actors[id]._type  = BOX;
}

void BulletInterface::createSphere( int id, double radius, double mass )
{
    btCollisionShape* sphereShape = new btSphereShape( radius );
    btTransform sphereTransform;
	sphereTransform.setIdentity();
	
	btVector3 localInertia(0.0, 0.0, 0.0);
	if ( mass>0.0 )
	    sphereShape->calculateLocalInertia( mass, localInertia );
	
    btDefaultMotionState* motionState = new btDefaultMotionState(sphereTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( mass, motionState, sphereShape, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body );
    body->setFriction(10.3f);
    body->setRestitution(0.1f);
    _actors[id]._body  = body;
    _actors[id]._type  = SPHERE;
}

static inline void add_wheel(btRaycastVehicle* v, osg::Vec3 connection_point, const double radius, btRaycastVehicle::btVehicleTuning tuning_, const bool is_front)
{
	btWheelInfo& wi = v->addWheel(phys::to_bullet_vector3(connection_point),btVector3(0,0,-1),btVector3(1,0,0), 0.0f,btScalar(radius),tuning_,!is_front);
	wi.m_suspensionStiffness = 20.f;
	wi.m_wheelsDampingRelaxation = 2.3;
	wi.m_wheelsDampingCompression = 4.4;
	wi.m_frictionSlip = 10.;
	wi.m_rollInfluence = 0.1f;
}


void BulletInterface::createUFO(osg::Node* node,int id, double mass)
{	
	btTransform cmt(btQuaternion(0,0,0),btVector3(1,0,0));
	cmt.setIdentity();
	////localTrans effectively shifts the center of mass with respect to the chassis
	//cmt.setOrigin(btVector3(0,-10,0));	
	
	wheels_info_t wi;
    aircraft::params_t p;
    p.mass     = mass;	
    double shift;
    btCompoundShape*  s = fill_cs(node,wi,p,shift);
	
	btTransform  tr;
	tr.setIdentity();
	btVector3 aabbMin,aabbMax;
	s->getAabb(tr,aabbMin,aabbMax);

	btScalar dxx = btScalar(/*params_.wingspan*/(aabbMax.x() - aabbMin.x()) / 2);
	btScalar dyy = btScalar(/*params_.length*/(aabbMax.y() - aabbMin.y()) / 2);
	btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
	btScalar m12 = btScalar((/*params_.mass*/mass) /12);
	btVector3 localInertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

    
	//btVector3 localInertia(0.0, 0.0, 0.0);
	//if ( mass>0.0 )
	//	s->calculateLocalInertia( mass, localInertia );

	btDefaultMotionState* motionState = new btDefaultMotionState(cmt);
	btRigidBody::btRigidBodyConstructionInfo rigidInfo( mass, motionState, s, localInertia );
	btRigidBody* chassis = new btRigidBody(rigidInfo);

	btRaycastVehicle::btVehicleTuning    tuning_;
	tuning_.m_maxSuspensionTravelCm = 500;
    //chassis->applyTorque(btVector3(100,100,100));
	btRaycastVehicle* v = new btRaycastVehicle(tuning_,chassis,new btDefaultVehicleRaycaster(_scene.get()));
	v->setCoordinateSystem(0,2,1);


	for (auto it=wi.begin();it!=wi.end();++it)
	{
		const double radius = (*it).radius*.75;
		osg::Vec3 connection_point = (*it).trans_f_body.getTrans() + osg::Vec3(0,shift,0); 

		add_wheel(v,connection_point,radius,tuning_,(*it).front);
	}

    v->setSteeringValue(0.1,0);
	//v->applyEngineForce(500,1);
	//v->applyEngineForce(500,2);

	_scene->addRigidBody( chassis );
	_scene->addVehicle(v);

	chassis->applyCentralForce(btVector3(0,50000,0));

	//chassis->setFriction(10.3f);
	//chassis->setRestitution(0.1f);
	_actors[id]._body  = chassis;
}

aircraft::info_ptr BulletInterface::createUFO2(osg::Node* node,int id, double mass)
{
	wheels_info_t wi;
	aircraft::params_t p;
	p.mass     = mass;
    double      shift;
	aircraft::control_ptr ctrl = std::make_shared<aircraft::impl>(shared_from_this(),phys::compound_shape_proxy(fill_cs(node,wi,p,shift)),p,decart_position());
	_actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();
	
	for (auto it=wi.begin();it!=wi.end();++it)
	{
		const double radius = (*it).radius*.75;
		osg::Vec3 connection_point = (*it).trans_f_body.getTrans() + osg::Vec3(0,shift,0); 

		ctrl->add_wheel(0,0,radius,connection_point,phys::cpr(),false,(*it).front);
	}

    _actors[id]._body->applyCentralForce(btVector3(0,500000,0));
    ctrl->set_steer(5);

	return ctrl;
}

void BulletInterface::createShape(osg::Node* node,int id, double mass)
{
    btCollisionShape* cs = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( node );
    
	btQuaternion quat;

    quat.setEuler(cg::pi_2f,cg::pi_2f,cg::pi_2f);

    btTransform comTransform; 
    comTransform.setIdentity();
    comTransform.setOrigin(btVector3(0, 0, 1));
    comTransform.setRotation(quat);

    btVector3 localInertia(0.0, 0.0, 0.0);
    if ( mass>0.0 )
        cs->calculateLocalInertia( mass, localInertia );


    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform::getIdentity());
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( mass, motionState, cs, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    
    // body->setAngularVelocity( btVector3( 0., .9, 0. ) );
    _scene->addRigidBody( body );
    _actors[id]._body  = body;
    _actors[id]._type  = SHAPE;
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
        actor->/*setWorldTransform*/setCenterOfMassTransform( trans );
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
	for (auto it = rigid_bodies_.begin(); it != rigid_bodies_.end(); ++it)
		(*it)->pre_update(step);

    _scene->stepSimulation( btScalar(step), 10, btScalar(0.01) );
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

phys::bt_dynamics_world_ptr BulletInterface::dynamics_world() const
{
	return _scene;
}

phys::bt_vehicle_raycaster_ptr BulletInterface::vehicle_raycaster() const
{
	return vehicle_raycaster_;
}

void BulletInterface::register_rigid_body( rigid_body_impl * rb )
{
	rigid_bodies_.insert(rb);
}

void BulletInterface::unregister_rigid_body( rigid_body_impl * rb )
{
	rigid_bodies_.erase(rb);



}