#include "stdafx.h"

#include "../find_node_visitor.h"
#include "../high_res_timer.h"
#include "BulletInterface.h"
#include "../aircraft.h"                // FIXME TODO don't need here 
#include "../aircraft_common.h"
#include "../aircraft_shassis_impl.h"   // And maybe this too
#include "nodes_manager.h"
#include "../ada.h"
#include "../bada_import.h"
#include "../phys_aircraft.h"


#include "RigidUpdater.h"

namespace bi
{

    void RigidUpdater::addGround( const osg::Vec3& gravity )
    {
        _sys->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), gravity,
            [&](int id){
                if(_on_collision)
                    _on_collision(_physicsNodes[id].get());   
        } );

        //const bool debug( arguments.read( "--debug" ) );
        if( _debug )
        {
            //osg::notify( osg::INFO ) << "osgbpp: Debug" << std::endl;
            _dbgDraw = new avCollision::GLDebugDrawer();
            _dbgDraw->setDebugMode( ~btIDebugDraw::DBG_DrawText );
            if(_root->getNumParents()>0)
                _root->getParent(0)->addChild( _dbgDraw->getSceneGraph() );
            else
                _root->addChild( _dbgDraw->getSceneGraph() );
        }

        if(_dbgDraw)
            _sys->setDebugDrawer(_dbgDraw);
    }



    void RigidUpdater::addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        
        osg::ComputeBoundsVisitor cbv;
        node->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();

        float rot_angle = -90.f;
        auto tr = osg::Matrix::translate(osg::Vec3(0.0,0.0,-(ym)/2.0f));
        if(dynamic_cast<osg::LOD*>(node))
        {
            rot_angle = 0;
            tr = osg::Matrix::translate(osg::Vec3(0,0,-(zm)/2.0f));
        }

        osg::MatrixTransform* positioned = new osg::MatrixTransform(tr);

        const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
            osg::inDegrees(0.f) , osg::Y_AXIS,
            osg::inDegrees(0.f)   , osg::Z_AXIS ); 

        osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));

        rotated->addChild(node);
        positioned->addChild(rotated);
        osg::Vec3 half_length ( (bb.xMax() - bb.xMin())/2.0f,(bb.zMax() - bb.zMin())/2.0f,(bb.yMax() - bb.yMin()) /2.0f );
        if(dynamic_cast<osg::LOD*>(node))
        {
            half_length = osg::Vec3 ( (bb.xMax() - bb.xMin())/2.0f,(bb.yMax() - bb.yMin())/2.0f,(bb.zMax() - bb.zMin()) /2.0f );
        }

        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);

#if 0
        _sys->createBox( id, half_length, mass );
        addPhysicsData( id, positioned, pos, vel, mass );
#else  
        _sys->createShape(lod3, id, mass);
        addPhysicsData( id, rotated, pos, vel, mass );
#endif
    }

    void RigidUpdater::addUFO(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        int id = _physicsNodes.size();
        _sys->createUFO( lod3,id, mass );

        _sys->setMatrix( id, osg::Matrix::translate(pos) );
        _sys->setVelocity( id, vel );

    }

    //inline nodes_management::manager_ptr get_nodes_manager(osg::Node* node) 
    //{
    //    return nodes_management::create_manager(node);
    //}

    void RigidUpdater::addUFO2(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        int id = _physicsNodes.size();
        
        nm::manager_ptr man = nm::create_manager(lod3);
        
        size_t pa_size = _phys_aircrafts.size();
        //if(_phys_aircrafts.size()==0)
        {
            aircraft::phys_aircraft_ptr ac_ = aircraft::phys_aircraft_impl::create(
                              cg::geo_base_3(cg::geo_point_3(0.000* (pa_size+1),0.0*(pa_size+1),0)),
                                                                                   _sys,
                                                                                    man,
                geo_position(cg::geo_point_3(0.000* (pa_size+1),0.005*(pa_size+1),0),cg::quaternion(cg::cpr(0, 0, 0))),
                                                          ada::fill_data("BADA","A319"),                                                   
                                boost::make_shared<aircraft::shassis_support_impl>(man),
                                                                                     0);

             ac_->go_to_pos(cg::geo_point_3(0.004,0.0000,0),cg::quaternion(cg::cpr(45, 0, 0)));
            _phys_aircrafts.push_back(ac_);
        }

        _aircrafts.push_back(_sys->createUFO2( lod3,id, mass ));

        osg::ComputeBoundsVisitor cbv;
        node->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();

        float rot_angle = -90.f;
        auto tr = osg::Matrix::translate(osg::Vec3(0.0,-(ym)/2.0f,0.0));
        if(dynamic_cast<osg::LOD*>(node))
        {
            rot_angle = 0;
            tr = osg::Matrix::translate(osg::Vec3(0,0,-(zm)/2.0f));
        }        

        osg::MatrixTransform* positioned = new osg::MatrixTransform(tr);

        const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
            osg::inDegrees(0.f) , osg::Y_AXIS,
            osg::inDegrees(0.f)   , osg::Z_AXIS ); 

        osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
        positioned->addChild(rotated);
        rotated->addChild(node);

        addPhysicsData( id, positioned, pos, /*vel*/osg::Vec3(0.0,0.0,0.0), mass );
        phys::aircraft::control_ptr(_aircrafts.back())->apply_force(vel);
    }

    void RigidUpdater::addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        _sys->createBox( id, shape->getHalfLengths(), mass );
        addPhysicsData( id, shape, pos, vel, mass );
    }

    void RigidUpdater::addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        _sys->createSphere( id, shape->getRadius(), mass );
        addPhysicsData( id, shape, pos, vel, mass );
    }

    bool RigidUpdater::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = static_cast<osgViewer::View*>( &aa );
        if ( !view || !_root ) return false;

        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::KEYUP:
            if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Return )
            {
                osg::Vec3 eye, center, up, dir;
                view->getCamera()->getViewMatrixAsLookAt( eye, center, up );
                dir = center - eye; dir.normalize();
                addPhysicsSphere( new osg::Sphere(osg::Vec3(), 0.5f), eye, dir * 200.0f, 2.0 );
            } 
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Right )
            {
                double steer  = phys::aircraft::control_ptr(_aircrafts[0])->get_steer();
                steer = cg::bound(cg::norm180(/*desired_course - cur_course*/++steer),-65., 65.);
                phys::aircraft::control_ptr(_aircrafts[1])->set_steer(steer);  

            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Left )
            {
                double steer  = phys::aircraft::control_ptr(_aircrafts[0])->get_steer();
                steer = cg::bound(cg::norm180(/*desired_course - cur_course*/--steer),-65., 65.);
                phys::aircraft::control_ptr(_aircrafts[1])->set_steer(steer);
            }

            break;
        case osgGA::GUIEventAdapter::FRAME:
            {
                double dt = _hr_timer.get_delta();
                if( _dbgDraw)
                    _dbgDraw->BeginDraw();

                for(auto it = _phys_aircrafts.begin();it!=_phys_aircrafts.end();++it)
                    (*it)->update();

                _sys->update( dt );
                


                for ( NodeMap::iterator itr=_physicsNodes.begin();
                    itr!=_physicsNodes.end(); ++itr )
                {
                    osg::Matrix matrix = _sys->getMatrix(itr->first);
                    itr->second->setMatrix( matrix );
                }

                if( _dbgDraw)
                {
                    _sys->getScene()->debugDrawWorld();
                    _dbgDraw->EndDraw();
                }
            } 

            break;
        default: break;
        }
        return false;
    }


    void RigidUpdater::addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
        const osg::Vec3& vel, double /*mass*/ )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(shape) );

        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( geode.get() );
        _root->addChild( mt.get() );

        _sys->setMatrix( id, osg::Matrix::translate(pos) );
        _sys->setVelocity( id, vel );
        _physicsNodes[id] = mt;
    }

    void RigidUpdater::addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
        const osg::Vec3& vel, double /*mass*/ )
    {
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( node );
        _root->addChild( mt.get() );

        _sys->setMatrix( id, osg::Matrix::translate(pos) );
        _sys->setVelocity( id, vel );
        _physicsNodes[id] = mt;
    }


    void RigidUpdater::handlePointEvent(std::vector<cg::point_3> const &simple_route)
    {

    }

}