#include "stdafx.h"

#include "utils/high_res_timer.h"


#include <btBulletDynamicsCommon.h> 
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftBody.h>

#include "BulletInterface.h"

#include "bullet_helpers.h"

#include "sensor.h"

#include "aircraft_phys.h"
#include "aircraft_phys_lite.h"
#include "bvh_static_mesh.h"
#include "static_convex.h"
#include "ray_cast_vehicle.h"
#include "flock_child_phys.h"
#include "aerostat_phys.h"

#include "ada/ada.h"
#include "bada/bada_import.h"
#include "GLDebugDrawer.h"

bool loadBulletFile(std::string name, btCompoundShape*& trimeshShape);

#ifdef DEPRECATED
    #include "kernel/systems/systems_base.h"
    #include "kernel/systems/fake_system.h"
    #include "kernel/object_class.h"

    #include "nm/nodes_manager.h" // TODO FIXME убрать нафиг

    FIXME(Неправильное использование node_impl)
    #include "nm/node_impl.h"
#endif

FIXME("deprecated")
#include "osgbullet_helpers.h"

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
	void fill_cs(const std::string& model_name, /*osg::Node* node,*/ wheels_info_t& wi,phys::aircraft::params_t& p, compound_sensor_impl& cs )
    { 
#ifdef DEPRECATED
        if(model_name.empty())
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
		
			btCollisionShape* cs_body = nullptr;
			cs.cs_ = new btCompoundShape;

			sh_f->setNodeMask(0);
			sh_r_r->setNodeMask(0);
			sh_r_l->setNodeMask(0);
#if 0
            FIXME("При btTriMeshCollisionShape проваливаемся при столкновениях, btConvexTriMeshCollisionShape не сериализуется" )
            cs_body   = /*btConvexTriMeshCollisionShapeFromOSG*/btTriMeshCollisionShapeFromOSG( body );
#else
            cs_body = osgbCollision::btCompoundShapeFromOSGGeodes( body,CONVEX_HULL_SHAPE_PROXYTYPE,osgbCollision::Y,3 );
#endif
			sh_f->setNodeMask(/*0xffffffff*/0x00010000);
			sh_r_r->setNodeMask(/*0xffffffff*/0x00010000);
			sh_r_l->setNodeMask(/*0xffffffff*/0x00010000);

            if(cs_body)
			    cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_body);
		}
		else
#endif
		{
		   bool loaded = loadBulletFile(cfg().path.data + "/models/" + model_name + "/" + model_name + ".bullet",  cs.cs_);
           if(loaded)
           {
               const btTransform ct0 = cs.cs_->getChildTransform(0);
               cs.offset_ = from_bullet_vector3(ct0.getOrigin()); 
           }
           else
               cs.cs_ = nullptr;
		}
         
	}
}

namespace ray_cast_vehicle
{
    void fill_cs(const std::string& model_name, /*osg::Node* node,*/ wheels_info_t& wi, compound_sensor_impl& cs )
    {
#ifdef DEPRECATED
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

            auto wheels = findNodes(node,"wheel",findNodeVisitor::not_exact);

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
                (*it)->setNodeMask(/*0xffffffff*/0x00010000);


            cs.cs_->addChildShape(btTransform(btQuaternion(0,0,0),to_bullet_vector3(cs.offset_)),cs_body);
        }
        else
#endif
        {
            bool loaded = loadBulletFile(cfg().path.data + "/models/" + model_name + "/" + model_name + ".bullet",  cs.cs_);
            if(loaded)
            {
                const btTransform ct0 = cs.cs_->getChildTransform(0);
                cs.offset_ = from_bullet_vector3(ct0.getOrigin()); 
            }
            else
                cs.cs_ = nullptr;
        }

    }

}

namespace flock
{
    void fill_cs(const std::string& model_name, compound_sensor_impl& cs )
    { 

        bool loaded = loadBulletFile(cfg().path.data + "/models/" + model_name + "/" + model_name + ".bullet",  cs.cs_);
        if(loaded)
        {
            const btTransform ct0 = cs.cs_->getChildTransform(0);
            cs.offset_ = from_bullet_vector3(ct0.getOrigin());
            FIXME(Размер гребаных птичек)
            cs.cs_->setLocalScaling( btVector3(0.01,0.01,0.01));
        }
        else
            cs.cs_ = nullptr;

    }
}

namespace aerostat
{
	void fill_cs(const std::string& model_name, compound_sensor_impl& cs )
	{ 

		bool loaded = loadBulletFile(cfg().path.data + "/models/" + model_name + "/" + model_name + ".bullet",  cs.cs_);
		if(loaded)
		{
			const btTransform ct0 = cs.cs_->getChildTransform(0);
			cs.offset_ = from_bullet_vector3(ct0.getOrigin());
				// cs.cs_->setLocalScaling( btVector3(0.01,0.01,0.01));
		}
		else
			cs.cs_ = nullptr;

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
			const std::string model_name = manager->get_model();

            ::aircraft::fill_cs(model_name, /*nm::node_impl_ptr(manager->get_node(0))->as_osg_node(),*/wi,p,cs);

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

            ::ray_cast_vehicle::fill_cs(model_name, /*nm::node_impl_ptr(manager->get_node(0))->as_osg_node(),*/wi,cs);

            return boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_);
        }
    }

    namespace flock
    {
        compound_sensor_ptr fill_cs(nm::manager_ptr manager)
        {
            compound_sensor_impl cs;
            const std::string model_name = manager->get_model();

            ::flock::fill_cs(model_name,cs);

            return boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_);
        }
    }

	namespace aerostat
	{
		compound_sensor_ptr fill_cs(nm::manager_ptr manager)
		{
			compound_sensor_impl cs;
			const std::string model_name = manager->get_model();

			::aerostat::fill_cs(model_name,cs);

			return boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_);
		}
	}
}

namespace phys
{

    typedef std::pair<const btRigidBody*, const btRigidBody*> CollisionPair;
    typedef std::set<CollisionPair>                           CollisionPairs;

    struct BulletInterface::_private
    {
        _private()
            :_dd (nullptr), _dw(nullptr)
        {}

        struct data
        {
            btRigidBody* _body;
            BulletInterface::object_t     _type;
        };

        typedef std::map<int, data>            ActorMap;
        ActorMap                              _actors;
        bt_dynamics_world_ptr                 _dw;   
        btDefaultCollisionConfiguration*      _configuration;
        btCollisionDispatcher*                _dispatcher;
        btBroadphaseInterface*                _overlappingPairCache;
        btSequentialImpulseConstraintSolver*  _solver;

        boost::scoped_ptr<btBroadphaseInterface>           broadphase_;
        boost::scoped_ptr<btCollisionDispatcher>           dispatcher_;
        boost::scoped_ptr<btConstraintSolver>              solver_;
        boost::scoped_ptr<btDefaultCollisionConfiguration> collision_configuration_;


        CollisionPairs                        _pairsLastUpdate;
        btIDebugDraw*                         _dd;

        bt_vehicle_raycaster_ptr              vehicle_raycaster_;
        std::vector<rigid_body_impl *>	      rigid_bodies_;
        
        void CollisionEvent(btRigidBody * pBody0, btRigidBody * pBody1 , on_collision_f  on_collision_) 
        {
            // find the two colliding objects

            for (auto it = _actors.begin();it!= _actors.end();++it)
            {
                if(pBody0 == it->second._body && it->second._type == SPHERE)
                {
                    if (on_collision_)
                        on_collision_(it->first);

                }

                if(pBody1 == it->second._body)
                {

                }
            }
        }
    };
 

	static void internal_tick_callback(btDynamicsWorld *world, btScalar /*timeStep*/);
    static bool contact_added_callback (btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0,
        int partId0, int index0,
        const btCollisionObjectWrapper* colObj1,
        int partId1, int index1);


BulletInterface::BulletInterface()
    : on_collision_(nullptr)
    , base_(::get_base())
    , d_(new _private())
{

    d_->_configuration = new btDefaultCollisionConfiguration;
    d_->_dispatcher = new btCollisionDispatcher( d_->_configuration );
    d_->_overlappingPairCache = new btDbvtBroadphase;
    d_->_solver = new btSequentialImpulseConstraintSolver;

#if 0
    d_->_dw = boost::make_shared<btDiscreteDynamicsWorld>(d_->_dispatcher,d_->_overlappingPairCache, d_->_solver, d_->_configuration);
#endif
    
    
	d_->_dw = boost::make_shared<btSoftRigidDynamicsWorld>(d_->_dispatcher,d_->_overlappingPairCache, d_->_solver, d_->_configuration);
	
	d_->_dw->setGravity( btVector3(/*gravity[0], gravity[1], gravity[2]*/0,0,-9.8) );

    d_->_dw->getSolverInfo().m_numIterations = 20;
    d_->_dw->getSolverInfo().m_damping = 1.;
    d_->_dw->getSolverInfo().m_splitImpulse = false;
    d_->_dw->getSolverInfo().m_solverMode |= SOLVER_SIMD;

    d_->_dw->setInternalTickCallback(internal_tick_callback);    


    d_->vehicle_raycaster_.reset(new btDefaultVehicleRaycaster(&*d_->_dw));
}

BulletInterface::~BulletInterface()
{
    if ( d_->_dw )
    {
        for ( int i=d_->_dw->getNumCollisionObjects()-1; i>=0; --i )
        {
            btCollisionObject* obj = d_->_dw->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if ( body && body->getMotionState() )
                delete body->getMotionState();
            
            d_->_dw->removeCollisionObject( obj );
            delete obj;
        }

		d_->_dw.reset();
    }
    
    delete d_->_solver;
    delete d_->_overlappingPairCache;
    delete d_->_dispatcher;
    delete d_->_configuration;
    delete d_;
}

void     BulletInterface::setDebugDrawer(/*btIDebugDraw*/debug_render* dd)
{
    d_->_dd= new avCollision::GLDebugDrawer(dd);    
    if(d_->_dd)
        d_->_dw->setDebugDrawer(d_->_dd);
}

void  BulletInterface::debugDrawWorld()
{
     d_->_dw->debugDrawWorld();
}

#ifdef DEPRECATED
void BulletInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity, on_collision_f on_collision )
{
 //   d_->_dw = boost::make_shared<btDiscreteDynamicsWorld>(d_->_dispatcher,d_->_overlappingPairCache, d_->_solver, d_->_configuration);
 //   d_->_dw->setGravity( btVector3(gravity[0], gravity[1], gravity[2]) );

	//d_->_dw->getSolverInfo().m_numIterations = 20;
	//d_->_dw->getSolverInfo().m_damping = 1.;
	//d_->_dw->getSolverInfo().m_splitImpulse = false;
	//d_->_dw->getSolverInfo().m_solverMode |= SOLVER_SIMD;
	//
	//d_->_dw->setInternalTickCallback(internal_tick_callback);
    
    FIXME(Использование глобальных коллбеков не приветствуется даешь альтернативу)
    // gContactAddedCallback = contact_added_callback;

    osg::Vec3 norm = plane.getNormal();
    btCollisionShape* groundShape = new btStaticPlaneShape( btVector3(norm[0], norm[1], norm[2]), plane[3] );
    btTransform groundTransform;
	groundTransform.setIdentity();
	
    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( 0.0, motionState, groundShape, btVector3(0.0, 0.0, 0.0) );
    btRigidBody* body = new btRigidBody(rigidInfo);
    d_->_dw->addRigidBody( body );
    body->setFriction(1.3f); 
    body->setActivationState(DISABLE_SIMULATION);
    body->setRestitution(0.5f);
    body->setUserPointer(new rigid_body_user_info_t(rb_terrain));

    on_collision_ = on_collision;

	d_->vehicle_raycaster_.reset(new btDefaultVehicleRaycaster(&*d_->_dw));

}
#endif

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
    d_->_dw->addRigidBody( body); 
    d_->_actors[id]._body = body;
    d_->_actors[id]._type  = BOX;
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
    d_->_dw->addRigidBody( body );
    body->setFriction(10.3f);
    body->setRestitution(0.1f);
    d_->_actors[id]._body  = body;
    d_->_actors[id]._type  = SPHERE;
}


#ifdef DEPRECATED
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

    /*btCompoundShape*  s =*/ ::aircraft::fill_cs("",node,wi,p, cs);
	
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
	btRaycastVehicle* v = new btRaycastVehicle(tuning_,chassis,new btDefaultVehicleRaycaster(d_->_dw.get()));
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

	d_->_dw->addRigidBody( chassis );
	d_->_dw->addVehicle(v);

	chassis->applyCentralForce(btVector3(0,50000,0));

	//chassis->setFriction(10.3f);
	//chassis->setRestitution(0.1f);
	d_->_actors[id]._body  = chassis;
}
#endif

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

#ifdef DEPRECATED
aircraft::info_ptr BulletInterface::createUFO2(osg::Node* node,int id, double mass)
{
	wheels_info_t wi;
	aircraft::params_t p;
    compound_sensor_impl cs;

    ::aircraft::fill_cs("",node,wi,p,cs);

    p = fill_params( ada::fill_data("BADA","A319"));

	aircraft::control_ptr ctrl = boost::make_shared<aircraft::impl>(shared_from_this(),boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_),p,decart_position());
	d_->_actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();
	
	for (auto it=wi.begin();it!=wi.end();++it)
	{
		const double radius = (*it).radius*.75;
		osg::Vec3 connection_point = (*it).trans_f_body.getTrans() + to_osg_vector3(cs.offset_); 

		ctrl->add_wheel(0,0,radius,from_osg_vector3(connection_point),/*phys::*/cpr(),false,(*it).front);
	}

	return ctrl;
} 

ray_cast_vehicle::info_ptr BulletInterface::createVehicle(osg::Node* node,int id,double mass)
{
    wheels_info_t wi;
    compound_sensor_impl cs;

    ::ray_cast_vehicle::fill_cs("",node,wi,cs);

    ray_cast_vehicle::info_ptr info = boost::make_shared<ray_cast_vehicle::impl>(shared_from_this(),mass,boost::make_shared<compound_sensor_impl>(cs.cs_,cs.offset_),decart_position());
    d_->_actors[id]._body  = rigid_body_impl_ptr(info)->get_body().get();

    for (auto it=wi.begin();it!=wi.end();++it)
    {
        const double radius = (*it).radius*.75;
        cg::point_3 connection_point = from_osg_vector3((*it).trans_f_body.getTrans()) + cs.offset_; 

        info->add_wheel(30,0,radius,connection_point,cg::cpr(),false/*,(*it).front*/);
    }   
    
    //_actors[id]._body->applyCentralForce(btVector3(0,20000,0));

    return info;
}
#endif

void BulletInterface::registerBody(int id,phys::rigid_body_ptr ctrl)
{
    d_->_actors[id]._body  = rigid_body_impl_ptr(ctrl)->get_body().get();
}

void BulletInterface::registerBody(int id)
{
    FIXME(Больной вопрос меняем способ передачи информации)
    if(d_->rigid_bodies_.size() > 0) 
    d_->_actors[id]._body  = d_->rigid_bodies_.back()->get_body().get();
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
    d_->_dw->addRigidBody( body );
    d_->_actors[id]._body  = body;
    d_->_actors[id]._type  = SHAPE;
}

void BulletInterface::setVelocity( int id, const osg::Vec3& vec )
{
    btRigidBody* actor = d_->_actors[id]._body;
    if ( actor )
        actor->setLinearVelocity( btVector3(vec.x(), vec.y(), vec.z()) );
}

void BulletInterface::setMatrix( int id, const osg::Matrix& matrix )
{
    btRigidBody* actor = d_->_actors[id]._body;
    if ( actor )
    {
        btTransform trans;
        trans.setFromOpenGLMatrix( osg::Matrixf(matrix).ptr() );
        actor->/*setWorldTransform*/setCenterOfMassTransform( trans );  // И то и другое работает 
    }
}

osg::Matrix BulletInterface::getMatrix( int id )
{
    btRigidBody* actor = d_->_actors[id]._body;
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
	for (auto it = d_->rigid_bodies_.begin(); it != d_->rigid_bodies_.end(); ++it)
		(*it)->pre_update(step);
    
    if (!cg::eq_zero(step))
    {
        d_->_dw->stepSimulation( btScalar(step), /*10*/cfg().model_params.msys_step/cfg().model_params.bullet_step, btScalar(cfg().model_params.bullet_step) ); 
    }

    checkForCollisionEvents();
}


void BulletInterface::checkForCollisionEvents() {
	// keep a list of the collision pairs we
	// found during the current update
	CollisionPairs pairsThisUpdate;
	
	// iterate through all of the manifolds in the dispatcher
	for (int i = 0; i < d_->_dispatcher->getNumManifolds(); ++i) {
			
		// get the manifold
		btPersistentManifold* pManifold = d_->_dispatcher->getManifoldByIndexInternal(i);
			
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
			if (d_->_pairsLastUpdate.find(thisPair) == d_->_pairsLastUpdate.end()) {
				d_->CollisionEvent((btRigidBody*)pBody0, (btRigidBody*)pBody1, on_collision_);
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
	d_->_pairsLastUpdate = pairsThisUpdate;
}




phys::bt_dynamics_world_ptr BulletInterface::dynamics_world() const
{
	return d_->_dw;
}

phys::bt_vehicle_raycaster_ptr BulletInterface::vehicle_raycaster() const
{
	return d_->vehicle_raycaster_;
}

void BulletInterface::register_rigid_body( rigid_body_impl * rb )
{
	// rigid_bodies_.insert(rb);
    d_->rigid_bodies_.push_back(rb);
}

void BulletInterface::unregister_rigid_body( rigid_body_impl * rb )
{
	//rigid_bodies_.erase(rb);
    // FIXME  пока не удаляем
    FIXME("Нету удаления, а надо бы особенно для сомолей")
}

static_mesh_ptr BulletInterface::create_static_mesh( sensor_ptr s )
{
    return boost::make_shared<bvh_static_mesh>(shared_from_this(), s);
}

static_convex_ptr BulletInterface::create_static_convex( sensor_ptr s, point_3 const& pos, quaternion const& orien )
{
    return boost::make_shared<static_convex_impl>(shared_from_this(), s, pos, orien);
}

aircraft::info_ptr BulletInterface::create_aircraft(const phys::aircraft::params_t & p,compound_sensor_ptr s,const decart_position & pos)
{    	
    return boost::make_shared<aircraft::impl>(shared_from_this(),s,p,pos);
}

aircraft::info_ptr BulletInterface::create_aircraft_pl(const phys::aircraft::params_t & p,compound_sensor_ptr s,const decart_position & pos)
{    	
	return  boost::make_shared<aircraft_physless::impl>(shared_from_this(),s,p,pos);
}

ray_cast_vehicle::info_ptr BulletInterface::create_ray_cast_vehicle(double mass,phys::compound_sensor_ptr s,const decart_position & pos)
{    	
    return boost::make_shared<ray_cast_vehicle::impl>(shared_from_this(),mass,s,pos);
}

flock::info_ptr BulletInterface::create_flock_child(const phys::flock::params_t & p,compound_sensor_ptr s,const decart_position & pos)
{    	
	return  boost::make_shared<flock::impl>(shared_from_this(),s,p,pos);
}

aerostat::info_ptr BulletInterface::create_aerostat(const phys::aerostat::params_t  & p,compound_sensor_ptr s,const decart_position & pos)
{    	
	return  boost::make_shared<aerostat::impl>(shared_from_this(),s,p,pos);
}


boost::optional<double> BulletInterface::intersect_first(cg::point_3 const& p, cg::point_3 const& q) const
{
	d_->_dw->updateAabbs();
	d_->_dw->computeOverlappingPairs();

	btCollisionWorld::ClosestRayResultCallback callback(to_bullet_vector3(p), to_bullet_vector3(q));
	d_->_dw->rayTest(to_bullet_vector3(p), to_bullet_vector3(q), callback);

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

                if (rbB->rigid_body_kind() == rb_aircraft && rbA->rigid_body_kind() == rb_terrain ||
                    rbA->rigid_body_kind() == rb_aircraft && rbB->rigid_body_kind() == rb_terrain
                   )
                {
                     //btVector3 rel_vel = obB->getInterpolationLinearVelocity() - obA->getInterpolationLinearVelocity();
                     {
                         // force_log fl;

                         LOG_ODS_MSG( "  obB->getInterpolationLinearVelocity() length = " <<  obB->getInterpolationLinearVelocity().length() << "\n" <<
                                      "  obA->getInterpolationLinearVelocity() length = " <<  obA->getInterpolationLinearVelocity().length() << "\n"  
                             );

                     }
                }
			}
		}
	}  
}

btVector3 get_face_normal (const btStridingMeshInterface *mesh, int face) {
    PHY_ScalarType vertexes_type, indexes_type;
    const unsigned char *vertexes;
    int num_vertexes;
    int vertexes_stride;
    const unsigned char *indexes;
    int num_faces;
    int face_stride;
    mesh->getLockedReadOnlyVertexIndexBase(&vertexes, num_vertexes, vertexes_type, vertexes_stride,
        &indexes, face_stride, num_faces, indexes_type);
    assert(vertexes_type == PHY_FLOAT);
    assert(indexes_type == PHY_INTEGER);
    const int *indexes2 = reinterpret_cast<const int *>(indexes + face_stride*face);
    int i1=indexes2[0], i2=indexes2[1], i3=indexes2[2];
    btVector3 v1 = *reinterpret_cast<const btVector3 *>(vertexes + vertexes_stride * i1);
    btVector3 v2 = *reinterpret_cast<const btVector3 *>(vertexes + vertexes_stride * i2);
    btVector3 v3 = *reinterpret_cast<const btVector3 *>(vertexes + vertexes_stride * i3);
    btVector3 r;
    r = (v2-v1).cross(v3-v1);
    r.normalize();
    return r;
}

static void contact_added_callback_obj (btManifoldPoint& cp,
    const btCollisionObject* colObj,
    int partId, int index)
{
    (void) partId;
    (void) index;
    btCollisionShape const* shape = colObj->getCollisionShape();

    if (shape->getShapeType() != TRIANGLE_MESH_SHAPE_PROXYTYPE) 
        return;

    btTriangleMeshShape const* tshape = static_cast<const btTriangleMeshShape*>(shape);

    const btStridingMeshInterface *mesh = tshape->getMeshInterface();

    btVector3 face_normal = get_face_normal(mesh,index);
    float dot = face_normal.dot(cp.m_normalWorldOnB);
    cp.m_normalWorldOnB = dot > 0 ? face_normal : -face_normal;
}

static bool contact_added_callback (btManifoldPoint& cp,
    const btCollisionObjectWrapper* colObj0,
    int partId0, int index0,
    const btCollisionObjectWrapper* colObj1,
    int partId1, int index1)
{
    contact_added_callback_obj(cp, colObj0->getCollisionObject(), partId0, index0);
    contact_added_callback_obj(cp, colObj1->getCollisionObject(), partId1, index1);
    return true;
}


} // namespace phys

