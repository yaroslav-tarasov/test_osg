#include "stdafx.h"


#include <btBulletDynamicsCommon.h> 
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>
#include "../high_res_timer.h"
#include "aircraft_phys.h"
#include "sensor.h"
#include "BulletInterface.h"

#include "ray_cast_vehicle.h"

#include "ada/ada.h"
#include "bada_import.h"

#include "kernel/systems/systems_base.h"
#include "fake_system.h"
#include "kernel/object_class.h"

//#include "nodes_manager/nodes_manager.h" // TODO FIXME убрать нафиг
//#include "nodes_manager/node_impl.h"

bool loadBulletFile(std::string name, btCompoundShape*& trimeshShape);

#include "nm/nodes_manager.h" // TODO FIXME убрать нафиг
FIXME(Неправильное использование node_impl)
#include "nm/node_impl.h"

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



namespace phys
{
    BulletInterface*  sys()
    {
        static boost::shared_ptr<BulletInterface> inst = create();
        return inst.get();
    }
    
    system_ptr create_phys_system()
    {
       static boost::shared_ptr<BulletInterface> inst = create();
       return  inst;
    }
	
    boost::shared_ptr<BulletInterface> create()
	{
		return boost::make_shared<BulletInterface>();
	}
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
}

namespace aircraft
{
	void fill_cs(osg::Node* node, wheels_info_t& wi,phys::aircraft::params_t& p, compound_sensor_impl& cs )
    {          
        osg::ComputeBoundsVisitor cbv;
        node->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = abs(bb.xMax() - bb.xMin());
        float ym = abs(bb.yMax() - bb.yMin());
        float zm = abs(bb.zMax() - bb.zMin());
        
        float dx = abs(bb.xMax()) - xm / 2.f;
        float dy = abs(bb.yMax()) - ym / 2.f;
        float dz = abs(bb.zMax()) - zm / 2.f;

        cs.offset_ = cg::point_3(0,/*-zm/2*/-dz,0);
        //p.wingspan = xm;
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
		

		//btTransform cmt(btQuaternion(0,0,0),btVector3(1,0,0));
		//cmt.setIdentity();
		////localTrans effectively shifts the center of mass with respect to the chassis
		//cmt.setOrigin(btVector3(0,-10,0));	

		btCompoundShape*  s = cs.cs_ = new btCompoundShape;

		sh_f/*_wheel*/->setNodeMask(0);
		sh_r_r/*_wheel*/->setNodeMask(0);
		sh_r_l/*_wheel*/->setNodeMask(0);

		//btCollisionShape* cs_sh_r_l = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( sh_r_l );
		//btCollisionShape* cs_sh_r_r = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( sh_r_r );
		//btCollisionShape* cs_sh_f   = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( sh_f );
		btCollisionShape* cs_body   = osgbCollision::/*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );

		sh_f/*_wheel*/->setNodeMask(0xffffffff);
		sh_r_r/*_wheel*/->setNodeMask(0xffffffff);
		sh_r_l/*_wheel*/->setNodeMask(0xffffffff);

		//cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_sh_f);
		//cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_sh_r_l);
		//cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_sh_r_r);
		cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_body);
        
         
		// return s;
	}
}

namespace ray_cast_vehicle
{
    void fill_cs(const std::string& model_name,osg::Node* node, wheels_info_t& wi, compound_sensor_impl& cs )
    {          
        if(model_name.empty())
        {
            osg::Node* lod3 =  findFirstNode(node,"Lod3");

            osg::ComputeBoundsVisitor cbv;
            (lod3?lod3:node)->accept( cbv );
            const osg::BoundingBox& bb = cbv.getBoundingBox();

            float xm = bb.xMax() - bb.xMin();
            float ym = bb.yMax() - bb.yMin();
            float zm = bb.zMax() - bb.zMin();

            float dx = abs(bb.xMax()) - xm / 2.f;
            float dy = abs(bb.yMax()) - ym / 2.f;
            float dz = abs(bb.zMax()) - zm / 2.f;

            cs.offset_ = cg::point_3(0,/*lod3?-zm/2:*/0,0);

            auto body   = findFirstNode(lod3?lod3:node,"Body",findNodeVisitor::not_exact);

            auto wheels = findAllNodes(node,"wheel",findNodeVisitor::not_exact);

            for (auto it = wheels.begin();it != wheels.end();++it)
            {   
                if((*it)->asTransform()) // А потом они взяли и поименовали геометрию как трансформы, убил бы
                {
                    wheel_info wii((*it)->getBound().radius(),/*is_front*/false);
                    wii.trans_f_body = get_relative_transform(node,(*it)/*,body*/);
                    wi.push_back(wii);
                    (*it)->setNodeMask(0);
                }
            }

            btCompoundShape*  s = cs.cs_ = new btCompoundShape;

            btCollisionShape* cs_body   = osgbCollision::/*btConvexHullCollisionShapeFromOSG*//*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
        
            for (auto it = wheels.begin();it != wheels.end();++it)
                (*it)->setNodeMask(0xffffffff);


            cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_body);
        }
        else
        {
            bool r = loadBulletFile(cfg().path.data + "/models/" + model_name + "/" + model_name + ".osgb.bullet",  cs.cs_);
             
        }

    }

}


namespace phys
{
    namespace aircraft
    {

        compound_sensor_ptr fill_cs(nm::manager_ptr manager)
        {
            wheels_info_t wi;
            aircraft::params_t p;
            compound_sensor_impl cs;

            ::aircraft::fill_cs(nm::node_impl_ptr(manager->get_node(0))->as_osg_node(),wi,p,cs);

            return boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_);
        }

    }

    namespace ray_cast_vehicle
    {
        compound_sensor_ptr fill_cs(nm::manager_ptr manager)
        {
            wheels_info_t wi;
            compound_sensor_impl cs;
            const std::string model_name = manager->get_model();

            ::ray_cast_vehicle::fill_cs(model_name, nm::node_impl_ptr(manager->get_node(0))->as_osg_node(),wi,cs);

            return boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_);
        }
    }

}

namespace phys
{
	static void internal_tick_callback(btDynamicsWorld *world, btScalar /*timeStep*/);

#if 0
BulletInterface* BulletInterface::instance()
{
    static boost::shared_ptr<BulletInterface> s_registry = boost::make_shared<BulletInterface>();
    return s_registry.get();
}
#endif

BulletInterface::BulletInterface()
    : _dw(nullptr)
    , _on_collision(nullptr)
    , _dd (nullptr)
    , base_(::get_base())
{

    _configuration = new btDefaultCollisionConfiguration;
    _dispatcher = new btCollisionDispatcher( _configuration );
    _overlappingPairCache = new btDbvtBroadphase;
    _solver = new btSequentialImpulseConstraintSolver;
    
    
}

BulletInterface::~BulletInterface()
{
    if ( _dw )
    {
        for ( int i=_dw->getNumCollisionObjects()-1; i>=0; --i )
        {
            btCollisionObject* obj = _dw->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if ( body && body->getMotionState() )
                delete body->getMotionState();
            
            _dw->removeCollisionObject( obj );
            delete obj;
        }

		_dw.reset();
    }
    
    delete _solver;
    delete _overlappingPairCache;
    delete _dispatcher;
    delete _configuration;
}


void BulletInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity, on_collision_f on_collision )
{
    _dw = boost::make_shared<btDiscreteDynamicsWorld>(_dispatcher,_overlappingPairCache, _solver, _configuration);
    _dw->setGravity( btVector3(gravity[0], gravity[1], gravity[2]) );

	_dw->getSolverInfo().m_numIterations = 20;
	_dw->getSolverInfo().m_damping = 1.;
	_dw->getSolverInfo().m_splitImpulse = false;
	_dw->getSolverInfo().m_solverMode |= SOLVER_SIMD;
	
	_dw->setInternalTickCallback(internal_tick_callback);

    osg::Vec3 norm = plane.getNormal();
    btCollisionShape* groundShape = new btStaticPlaneShape( btVector3(norm[0], norm[1], norm[2]), plane[3] );
    btTransform groundTransform;
	groundTransform.setIdentity();
	
    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( 0.0, motionState, groundShape, btVector3(0.0, 0.0, 0.0) );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _dw->addRigidBody( body );
    body->setFriction(1.3f); 
    body->setActivationState(DISABLE_SIMULATION);
    body->setRestitution(0.5f);

    _on_collision = on_collision;
	


	vehicle_raycaster_.reset(new btDefaultVehicleRaycaster(&*_dw));

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
    _dw->addRigidBody( body); 
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
    _dw->addRigidBody( body );
    body->setFriction(10.3f);
    body->setRestitution(0.1f);
    _actors[id]._body  = body;
    _actors[id]._type  = SPHERE;
}

static inline void add_wheel(btRaycastVehicle* v, osg::Vec3 connection_point, const double radius, btRaycastVehicle::btVehicleTuning tuning_, const bool is_front)
{
	btWheelInfo& wi = v->addWheel(phys::osg_helpers::to_bullet_vector3(connection_point),btVector3(0,0,-1),btVector3(1,0,0), 0.0f,btScalar(radius),tuning_,!is_front);
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
    //double shift;
    compound_sensor_impl cs;

    /*btCompoundShape*  s =*/ ::aircraft::fill_cs(node,wi,p, cs);
	
	btTransform  tr;
	tr.setIdentity();
	btVector3 aabbMin,aabbMax;
	cs.cs_->getAabb(tr,aabbMin,aabbMax);

	btScalar dxx = btScalar(/*params_.wingspan*/(aabbMax.x() - aabbMin.x()) / 2);
	btScalar dyy = btScalar(/*params_.length*/(aabbMax.y() - aabbMin.y()) / 2);
	btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
	btScalar m12 = btScalar((/*params_.mass*/mass) /12);
	btVector3 localInertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

	//btVector3 localInertia(0.0, 0.0, 0.0);
	//if ( mass>0.0 )
	//	s->calculateLocalInertia( mass, localInertia );

	btDefaultMotionState* motionState = new btDefaultMotionState(cmt);
	btRigidBody::btRigidBodyConstructionInfo rigidInfo( mass, motionState, cs.cs_, localInertia );
	btRigidBody* chassis = new btRigidBody(rigidInfo);

	btRaycastVehicle::btVehicleTuning    tuning_;
	tuning_.m_maxSuspensionTravelCm = 500;
    //chassis->applyTorque(btVector3(100,100,100));
	btRaycastVehicle* v = new btRaycastVehicle(tuning_,chassis,new btDefaultVehicleRaycaster(_dw.get()));
	v->setCoordinateSystem(0,2,1);


	for (auto it=wi.begin();it!=wi.end();++it)
	{
		const double radius = (*it).radius*.75;
		osg::Vec3 connection_point = (*it).trans_f_body.getTrans() + to_osg_vector3(cs.offset_); 

		add_wheel(v,connection_point,radius,tuning_,(*it).front);
	}

    v->setSteeringValue(0.1,0);
	//v->applyEngineForce(500,1);
	//v->applyEngineForce(500,2);

	_dw->addRigidBody( chassis );
	_dw->addVehicle(v);

	chassis->applyCentralForce(btVector3(0,50000,0));

	//chassis->setFriction(10.3f);
	//chassis->setRestitution(0.1f);
	_actors[id]._body  = chassis;
}

aircraft::params_t fill_params( ada::data_t const & fsettings)
{
    const double phys_mass_factor_ = 1000; 

    phys::aircraft::params_t params;
    double const mass = fsettings.max_mass / phys_mass_factor_;
    double const S = fsettings.S / phys_mass_factor_;
    double const min_speed = fsettings.v_stall_ld* fsettings.c_v_min;
    double const cd_0 = 2*(fsettings.cd_0_landing + fsettings.cd_0_landing_gear);
    double const cd_2 = fsettings.cd_2_landing;
    double const air_density = 1.225;
    double const g = 9.8;
    double const Cl = fsettings.max_mass * g / (air_density * fsettings.S * min_speed * min_speed);

    params.mass = fsettings.max_mass / phys_mass_factor_;
    params.S = S;
    params.wingspan = fsettings.span;
    params.chord = fsettings.S / params.wingspan;
    params.length = fsettings.length; 
    params.Cl = Cl;
    params.Cd0 = cd_0;
    params.Cd2 = cd_2;
    params.ClAOA = 0.4;
    params.Cs = 0.2;
    params.thrust = (fsettings.ct_1 * (100. / fsettings.ct_2 + fsettings.ct_3 * 100. * 100.));

    return params;
}


aircraft::info_ptr BulletInterface::createUFO2(osg::Node* node,int id, double mass)
{
	wheels_info_t wi;
	aircraft::params_t p;
    compound_sensor_impl cs;

    ::aircraft::fill_cs(node,wi,p,cs);

    p = fill_params( ada::fill_data("BADA","A319"));

	aircraft::control_ptr ctrl = boost::make_shared<aircraft::impl>(shared_from_this(),boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_),p,decart_position());
	_actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();
	
	for (auto it=wi.begin();it!=wi.end();++it)
	{
		const double radius = (*it).radius*.75;
		osg::Vec3 connection_point = (*it).trans_f_body.getTrans() + to_osg_vector3(cs.offset_); 

		ctrl->add_wheel(0,0,radius,connection_point,phys::cpr(),false,(*it).front);
	}

	return ctrl;
} 

ray_cast_vehicle::info_ptr BulletInterface::createVehicle(osg::Node* node,int id,double mass)
{
    wheels_info_t wi;
    compound_sensor_impl cs;

    ::ray_cast_vehicle::fill_cs("",node,wi,cs);

    ray_cast_vehicle::info_ptr info = boost::make_shared<ray_cast_vehicle::impl>(shared_from_this(),mass,boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_),decart_position());
    _actors[id]._body  = rigid_body_impl_ptr(info)->get_body().get();

    for (auto it=wi.begin();it!=wi.end();++it)
    {
        const double radius = (*it).radius*.75;
        cg::point_3 connection_point = from_osg_vector3((*it).trans_f_body.getTrans()) + cs.offset_; 

        info->add_wheel(30,0,radius,connection_point,cg::cpr(),false/*,(*it).front*/);
    }   
    
    //_actors[id]._body->applyCentralForce(btVector3(0,20000,0));

    return info;
}


void BulletInterface::registerBody(int id,phys::rigid_body_ptr ctrl)
{
    _actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();
}

void BulletInterface::registerBody(int id)
{
    FIXME(Больной вопрос меняем способ передачи информации)
    if(rigid_bodies_.size() > 0) 
    _actors[id]._body  = rigid_bodies_.back()->get_body().get();
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
    _dw->addRigidBody( body );
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
        actor->/*setWorldTransform*/setCenterOfMassTransform( trans );  // И то и другое работает 
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

void BulletInterface::update( double step )
{
	for (auto it = rigid_bodies_.begin(); it != rigid_bodies_.end(); ++it)
		(*it)->pre_update(step);
    
    if (!cg::eq_zero(step))
    {
        _dw->stepSimulation( btScalar(step), 10, btScalar(0.01) );
    }

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
	return _dw;
}

phys::bt_vehicle_raycaster_ptr BulletInterface::vehicle_raycaster() const
{
	return vehicle_raycaster_;
}

void BulletInterface::register_rigid_body( rigid_body_impl * rb )
{
	// rigid_bodies_.insert(rb);
    rigid_bodies_.push_back(rb);
}

void BulletInterface::unregister_rigid_body( rigid_body_impl * rb )
{
	// rigid_bodies_.erase(rb);
    // FIXME  пока не удаляем
    FIXME("Нету удаления, а надо бы особенно для сомолей")
}

aircraft::info_ptr BulletInterface::create_aircraft(const phys::aircraft::params_t & p,compound_sensor_ptr s,const decart_position & pos)
{    	
    aircraft::control_ptr ctrl = boost::make_shared<aircraft::impl>(shared_from_this(),s,p,pos);
    // FIXME TODO
    // _actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();

    return ctrl;
}

ray_cast_vehicle::info_ptr BulletInterface::create_ray_cast_vehicle(double mass,phys::compound_sensor_ptr s,const decart_position & pos)
{    	
    ray_cast_vehicle::control_ptr ctrl = boost::make_shared<ray_cast_vehicle::impl>(shared_from_this(),mass,s,pos);
    // FIXME TODO
    // _actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();

    return ctrl;
}

boost::optional<double> BulletInterface::intersect_first(cg::point_3 const& p, cg::point_3 const& q) const
{
	_dw->updateAabbs();
	_dw->computeOverlappingPairs();

	btCollisionWorld::ClosestRayResultCallback callback(to_bullet_vector3(p), to_bullet_vector3(q));
	_dw->rayTest(to_bullet_vector3(p), to_bullet_vector3(q), callback);

	if (!callback.hasHit())
		return boost::none;

	return callback.m_closestHitFraction;
}

system_ptr   BulletInterface::get_system(size_t zone)
{
       return shared_from_this();
}

optional<size_t>  BulletInterface::get_zone(cg::geo_point_3 const & pos) const
{
     FIXME(Магическое число)
     return 1;
}

optional<size_t>  BulletInterface::get_zone(std::string const& airport) const
{
    FIXME(Магическое число)  
    return 1;
}

cg::geo_base_3 const& BulletInterface::get_base(size_t zone) const
{
    return base_;
}

std::string BulletInterface::zone_name(size_t id) const
{
    return "name";
}

static void internal_tick_callback(btDynamicsWorld *world, btScalar /*timeStep*/)
{
	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject const* obA = static_cast<btCollisionObject const*>(contactManifold->getBody0());
		btCollisionObject const* obB = static_cast<btCollisionObject const*>(contactManifold->getBody1());

		if (obA->getUserPointer() && obB->getUserPointer())
		{
			rigid_body_user_info_t * rbA = (rigid_body_user_info_t *)(obA->getUserPointer());
			rigid_body_user_info_t * rbB = (rigid_body_user_info_t *)(obB->getUserPointer());

			size_t numContacts = (size_t)contactManifold->getNumContacts();
			if (numContacts)
			{
				if (rbA->rigid_body_kind() == rb_aircraft)
				{
					for (size_t i = 0; i < numContacts; ++i)
					{
						btManifoldPoint const& pnt = contactManifold->getContactPoint(i);
						btVector3 rel_vel = obA->getInterpolationLinearVelocity() - obB->getInterpolationLinearVelocity();
						// TODO : use angular speed

						dynamic_cast<rigid_body_impl*>(rbA)->has_contact(rbB, from_bullet_vector3(pnt.m_localPointA), from_bullet_vector3(rel_vel));
					}
				}

				if (rbB->rigid_body_kind() == rb_aircraft)
				{
					for (size_t i = 0; i < numContacts; ++i)
					{
						btManifoldPoint const& pnt = contactManifold->getContactPoint(i);
						btVector3 rel_vel = obB->getInterpolationLinearVelocity() - obA->getInterpolationLinearVelocity();
						// TODO : use angular speed

						dynamic_cast<rigid_body_impl*>(rbB)->has_contact(rbA, from_bullet_vector3(pnt.m_localPointB), from_bullet_vector3(rel_vel));
					}
				}
			}
		}
	}  
}




} // namespace phys

