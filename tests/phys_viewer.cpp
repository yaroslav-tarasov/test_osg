/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * osgBullet is (C) Copyright 2009-2012 by Kenneth Mark Bryden
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include "stdafx.h"


#include <osgbDynamics/GroundPlane.h>
#include <osgbCollision/GLDebugDrawer.h>
#include <osgbCollision/Utils.h>
#include <osgbInteraction/DragHandler.h>
#include <osgbInteraction/LaunchHandler.h>
#include <osgbInteraction/SaveRestoreHandler.h>

#include <osgwTools/Shapes.h>

#include <btBulletDynamicsCommon.h>

#include "BulletDynamics/Character/btCharacterControllerInterface.h"
#include "DynamicCharacterController.h"

#include "utils/visitors/ComputeTriMeshVisitor.h"

#include "av/avCore/Object.h"

namespace phys {
    bool loadBulletFile(std::string name, btCompoundShape*& trimeshShape);
}

btDiscreteDynamicsWorld* initPhysics()
{
    osg::Plane plane(0.0f, 0.0f, 1.0f, 0.0f);
    
    btBroadphaseInterface*                _overlappingPairCache (new btDbvtBroadphase);
    btDefaultCollisionConfiguration*      _configuration = new btDefaultCollisionConfiguration;
    btCollisionDispatcher*                _dispatcher = new btCollisionDispatcher( _configuration );
    btSequentialImpulseConstraintSolver*  _solver= new btSequentialImpulseConstraintSolver;

    btDiscreteDynamicsWorld* dynamicsWorld= new btDiscreteDynamicsWorld(_dispatcher,_overlappingPairCache, _solver, _configuration);
    dynamicsWorld->setGravity( btVector3(0, 0, -9.8) );

    dynamicsWorld->getSolverInfo().m_numIterations = 20;
    dynamicsWorld->getSolverInfo().m_damping = 1.;
    dynamicsWorld->getSolverInfo().m_splitImpulse = false;
    dynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_SIMD;
    //
    // dynamicsWorld->setInternalTickCallback(internal_tick_callback);
    
    osg::Vec3 norm = plane.getNormal();
    btCollisionShape* groundShape = new btStaticPlaneShape( btVector3(norm[0], norm[1], norm[2]), plane[3] );
    btTransform groundTransform;
    groundTransform.setIdentity();

    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( 0.0, motionState, groundShape, btVector3(0.0, 0.0, 0.0) );
    btRigidBody* body = new btRigidBody(rigidInfo);
    dynamicsWorld->addRigidBody( body );
    body->setFriction(/*1.3f*/0.99f); 
    body->setActivationState(DISABLE_SIMULATION);
    body->setRestitution(/*0.5f*/0.1f);
    // body->setUserPointer(new rigid_body_user_info_t(rb_terrain));

    return( dynamicsWorld );
}

void addRigidBody( btDiscreteDynamicsWorld* dynamics_world_, btCompoundShape*        chassis_shape_ )
{
    btTransform  tr;
    tr.setIdentity();
    btVector3 aabbMin,aabbMax;
    chassis_shape_->getAabb(tr,aabbMin,aabbMax);
    auto mass = 50.f;
    btScalar dxx = btScalar((aabbMax.x() - aabbMin.x()) / 2);
    btScalar dyy = btScalar((aabbMax.y() - aabbMin.y()) / 2);
    btScalar dzz = btScalar((aabbMax.z() - aabbMin.z()) / 2);
    btScalar m12 = btScalar((mass) /12);
    btVector3 inertia = m12 * btVector3(dyy*dyy + dzz*dzz, dxx*dxx + dzz*dzz, dyy*dyy + dxx*dxx);

    // btDefaultMotionState* motionState = new btDefaultMotionState(tr);
    
    btDefaultMotionState* motionState =
        new btDefaultMotionState(btTransform(btQuaternion(0, /*cg::pi/2*/0, 0), btVector3(0, 0, 0)));
    
    btRigidBody::btRigidBodyConstructionInfo chassis_construction_info(btScalar(mass), motionState, chassis_shape_, inertia);

    
    dynamics_world_->addRigidBody(new btRigidBody(chassis_construction_info));
}

int main_phys_viewer( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    const bool debugDisplay( arguments.find( "--debug" ) > 0 );

    btDiscreteDynamicsWorld* bw = initPhysics();
    osg::Group* root = new osg::Group;

    osg::Group* launchHandlerAttachPoint = new osg::Group;
    root->addChild( launchHandlerAttachPoint );
    
    std::string         model_name = "human";
    
    auto obj = avCore::createObject(model_name);
    osg::ref_ptr< osg::Node > rootModel( obj->getOrCreateNode() );
    if( !rootModel.valid() )
    {
        osg::notify( osg::FATAL ) << "mesh: Can't create mesh." << std::endl;
        return( 1 );
    }
    
    btCompoundShape*        cs_;
    bool loaded = phys::loadBulletFile(cfg().path.data + "/models/" + model_name + "/" + model_name + ".bullet",  cs_);
    if(loaded)
    {
        const btTransform ct0 = cs_->getChildTransform(0);
        // cs.offset_ = from_bullet_vector3(ct0.getOrigin()); 
        cs_->setLocalScaling( btVector3(0.013,0.013,0.013));
        addRigidBody( bw, cs_ );
    }

    root->addChild( rootModel.get() );
    
    DynamicCharacterController* m_character = new DynamicCharacterController ();
    m_character->setup();
    bw->addAction(m_character);

#if 0
    // Add ground
    const osg::Vec4 plane( 0., 0., 1., -100. );
    root->addChild( osgbDynamics::generateGroundPlane( plane, bw ) );
#endif


    osgbCollision::GLDebugDrawer* dbgDraw( NULL );
    if( /*debugDisplay*/ true)
    {
        dbgDraw = new osgbCollision::GLDebugDrawer();
        dbgDraw->setDebugMode( ~btIDebugDraw::DBG_DrawText );
        bw->setDebugDrawer( dbgDraw );
        root->addChild( dbgDraw->getSceneGraph() );
    }

    osgViewer::Viewer viewer( arguments );
	viewer.apply(new osgViewer::SingleScreen(1));
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    //viewer.setUpViewInWindow( 30, 30, 768, 480 );
    viewer.setSceneData( root );

    osgGA::TrackballManipulator* tb = new osgGA::TrackballManipulator;
    // tb->setHomePosition( osg::Vec3( 0., -16., 6. ), osg::Vec3( 0., 0., 5. ), osg::Vec3( 0., 0., 1. ) ); 
    viewer.setCameraManipulator( tb );
    viewer.getCamera()->setClearColor( osg::Vec4( .5, .5, .5, 1. ) );
    viewer.realize();


    double prevSimTime = 0.;
    while( !viewer.done() )
    {
        if( dbgDraw != NULL )
            dbgDraw->BeginDraw();

        const double currSimTime = viewer.getFrameStamp()->getSimulationTime();
        bw->stepSimulation( currSimTime - prevSimTime/*0.0*//*0.1*/ );
        prevSimTime = currSimTime;

        if( dbgDraw != NULL )
        {
            bw->debugDrawWorld();
            dbgDraw->EndDraw();
        }

        // worldInfo.m_sparsesdf.GarbageCollect();

        viewer.frame();
    }

    return( 0 );
}


AUTO_REG(main_phys_viewer)