#include "stdafx.h"

#include "BulletMTInterface.h"


#include <osgbDynamics/RigidBody.h>
#include <osgbDynamics/MotionState.h>
#include <osgbDynamics/GroundPlane.h>
#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/RefBulletObject.h>
#include <osgbCollision/Utils.h>
#include <osgbDynamics/TripleBuffer.h>
#include <osgbDynamics/PhysicsThread.h>
#include <osgbInteraction/DragHandler.h>
#include <osgbInteraction/LaunchHandler.h>
#include <osgbInteraction/SaveRestoreHandler.h>

#ifndef _DEBUG
#pragma comment(lib, "osgwTools.lib")
#pragma comment(lib, "osgbDynamics.lib")
#pragma comment(lib, "osgbInteraction.lib")
#pragma comment(lib, "osgbCollision.lib")
#else 
#pragma comment(lib, "osgwToolsd.lib")
#pragma comment(lib, "osgbDynamicsd.lib")
#pragma comment(lib, "osgbInteractiond.lib")
#pragma comment(lib, "osgbCollisiond.lib")
#endif



namespace bi
{

struct BulletMTInterface::private_data : public osg::Referenced
{  
    private_data()
        : _srh(new osgbInteraction::SaveRestoreHandler)
        , _pt (nullptr)
    {}

    ~private_data()
    {
        if (_pt)
            delete _pt;
    }

    btDiscreteDynamicsWorld*                            _scene;
    btDefaultCollisionConfiguration*                    _configuration;
    btCollisionDispatcher*                              _dispatcher;
    btBroadphaseInterface*                              _overlappingPairCache;
    btSequentialImpulseConstraintSolver*                _solver;
    osg::ref_ptr< osgbInteraction::SaveRestoreHandler > _srh;
    osgbDynamics::PhysicsThread*                        _pt;
    osgbDynamics::TripleBuffer                          _tBuf;
    osgbDynamics::MotionStateList                       _msl;
    osgbInteraction::LaunchHandler*                     _lh;
};

osgGA::GUIEventHandler* BulletMTInterface::getSaveRestoreHandler()
{
    return d->_srh.get();
}

BulletMTInterface* BulletMTInterface::instance()
{
    static osg::ref_ptr<BulletMTInterface> s_registry = new BulletMTInterface;
    return s_registry.get();
}

BulletMTInterface::BulletMTInterface()
    : d(new private_data)
{
    // Increase triple buffer size to hold lots of transform data.
    d->_tBuf.resize( 16384 );

    d->_configuration = new btDefaultCollisionConfiguration;
    d->_dispatcher = new btCollisionDispatcher( d->_configuration );
    d->_overlappingPairCache = new btDbvtBroadphase;
    d->_solver = new btSequentialImpulseConstraintSolver;
}


// osg::Transform*
void   BulletMTInterface::makeModel( const std::string& fileName, const int index, osg::Vec3 pos,const osg::Matrix& m, osg::Transform* amt  )
{
    osg::ref_ptr< osgbDynamics::CreationRecord > cr = new osgbDynamics::CreationRecord;
    cr->_sceneGraph = amt/*.get()*/;
    cr->_shapeType = BOX_SHAPE_PROXYTYPE;
    cr->_mass = .2f;
    cr->_restitution = 0.3f;
    cr->_parentTransform = m;
    btRigidBody* rb = osgbDynamics::createRigidBody( cr.get() );

    rb->setActivationState( DISABLE_DEACTIVATION );


    // Set up for multithreading and triple buffering.
    osgbDynamics::MotionState* motion = static_cast< osgbDynamics::MotionState* >( rb->getMotionState() );
    motion->registerTripleBuffer( &d->_tBuf );
    d->_msl.insert( motion );

    std::ostringstream ostr;
    ostr << fileName << index;
    d->_srh->add( ostr.str(), rb );

    amt->setUserData( new osgbCollision::RefRigidBody( rb ) );
    d->_scene->addRigidBody( rb );

    // return( amt.release() );
}

void BulletMTInterface::createCow( osg::Node* node, osg::Vec3 pos,const osg::Matrix& m, osg::Transform* amt )
{
    btCollisionShape* cs = osgbCollision::btConvexTriMeshCollisionShapeFromOSG( node );
    osgbDynamics::MotionState* motion = new osgbDynamics::MotionState();

    motion->setTransform( amt );
    motion->setParentTransform( m );
    btScalar mass( 2. );
    btVector3 inertia( 0, 0, 0 );
    cs->calculateLocalInertia( mass, inertia );
    btRigidBody::btRigidBodyConstructionInfo rb( mass, motion, cs, inertia );

    // Set up for multithreading and triple buffering.
    motion->registerTripleBuffer( &d->_tBuf );
    d->_msl.insert( motion );

    btRigidBody* body = new btRigidBody( rb );
    body->setActivationState( DISABLE_DEACTIVATION );
    d->_scene->addRigidBody( body );

    d->_srh->add( "cow", body );
    amt->setUserData( new osgbCollision::RefRigidBody( body ) );

}

void BulletMTInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity, on_collision_f on_collision )
{
    d->_scene = new btDiscreteDynamicsWorld( d->_dispatcher, d->_overlappingPairCache, d->_solver, d->_configuration );
    d->_scene->setGravity( btVector3(gravity[0], gravity[1], gravity[2]) );

    // static osgbDynamics::PhysicsThread pt( d->_scene, &tBuf );
    d->_pt = new osgbDynamics::PhysicsThread( d->_scene, &d->_tBuf );

    osg::Vec3 norm = plane.getNormal();
    btCollisionShape* groundShape = new btStaticPlaneShape( btVector3(norm[0], norm[1], norm[2]), plane[3] );
    btTransform groundTransform;
    groundTransform.setIdentity();

    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( 0.0, motionState, groundShape, btVector3(0.0, 0.0, 0.0) );
    btRigidBody* body = new btRigidBody(rigidInfo);
    d->_scene->addRigidBody( body );

    body->setActivationState(DISABLE_SIMULATION);

    on_collision_ = on_collision;

}

void BulletMTInterface::simulate()
{
    TripleBufferMotionStateUpdate( d->_msl, &d->_tBuf );
}

void BulletMTInterface::registerHandlers(osg::Group* attachPoint,osgViewer::Viewer* viewer)
{
     d->_lh =  new osgbInteraction::LaunchHandler(d->_scene,attachPoint,viewer->getCamera());

     {
         // Use a custom launch model: Sphere with radius 0.5 (instead of default 1.0)
         const float radius = 0.5f;
         osg::Geode* geode = new osg::Geode;
         geode->addDrawable( new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(), radius)) );

         d->_lh->setLaunchModel( geode, new btSphereShape( radius ) );
         d->_lh->setInitialVelocity( 50. );

         viewer->addEventHandler( d->_lh );
     }

     d->_srh->setLaunchHandler( d->_lh );
     d->_srh->capture();
     viewer->addEventHandler( d->_srh.get() );
     osgbInteraction::DragHandler* dh = new osgbInteraction::DragHandler(
         d->_scene, viewer->getCamera() );
     viewer->addEventHandler( dh );

     d->_lh->setThreadedPhysicsSupport( d->_pt, &d->_tBuf, &d->_msl );
     d->_srh->setThreadedPhysicsSupport( d->_pt );
     dh->setThreadedPhysicsSupport( d->_pt );


}

}