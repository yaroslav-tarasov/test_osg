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
#include "../vehicle.h"

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

    void RigidUpdater::addUFO2(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        int id = _physicsNodes.size();
        
        //nm::manager_ptr man = nm::create_manager(lod3);
        //
        //size_t pa_size = _phys_aircrafts.size();
        ////if(_phys_aircrafts.size()==0)
        //{
        //    aircraft::phys_aircraft_ptr ac_ = aircraft::phys_aircraft_impl::create(
        //                      cg::geo_base_3(cg::geo_point_3(0.000* (pa_size+1),0.0*(pa_size+1),0)),
        //                                                                           _sys,
        //                                                                            man,
        //        geo_position(cg::geo_point_3(0.000* (pa_size+1),0.005*(pa_size+1),0),cg::quaternion(cg::cpr(0, 0, 0))),
        //                                                  ada::fill_data("BADA","A319"),                                                   
        //                        boost::make_shared<aircraft::shassis_support_impl>(man),
        //                                                                             0);

        //    _phys_aircrafts.push_back(ac_);
        //}

        _aircrafts.push_back(_sys->createUFO2( lod3,id, mass ));

        //osg::ComputeBoundsVisitor cbv;
        //node->accept( cbv );
        //const osg::BoundingBox& bb = cbv.getBoundingBox();

        //float xm = bb.xMax() - bb.xMin();
        //float ym = bb.yMax() - bb.yMin();
        //float zm = bb.zMax() - bb.zMin();

        //float rot_angle = -90.f;
        //auto tr = osg::Matrix::translate(osg::Vec3(0.0,-(ym)/2.0f,0.0));
        //if(dynamic_cast<osg::LOD*>(node))
        //{
        //    rot_angle = 0;
        //    tr = osg::Matrix::translate(osg::Vec3(0,0,-(zm)/2.0f));
        //}        

        //osg::MatrixTransform* positioned = new osg::MatrixTransform(tr);

        //const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
        //    osg::inDegrees(0.f) , osg::Y_AXIS,
        //    osg::inDegrees(0.f)   , osg::Z_AXIS ); 

        //osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
        //positioned->addChild(rotated);
        //rotated->addChild(node);

        addPhysicsData( id, addGUIObject(node), pos, /*vel*/osg::Vec3(0.0,0.0,0.0), mass );
        phys::aircraft::control_ptr(_aircrafts.back())->apply_force(vel);
    }


    void RigidUpdater::addUFO3(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {
        // TODO FIXME И тут можно обдумать процесс управления всеми лодами сразу

        //osg::Node*  lod0 =  findFirstNode(node,"Lod0",findNodeVisitor::not_exact);
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);

        int id = _physicsNodes.size();

        nm::manager_ptr man = nm::create_manager(lod3);

        aircraft::shassis_support_ptr s = boost::make_shared<aircraft::shassis_support_impl>(nm::create_manager(node));

        size_t pa_size = _phys_aircrafts.size();
        aircraft::phys_aircraft_ptr ac_ = aircraft::phys_aircraft_impl::create(
            get_base(),
            _sys,
            man,
            geo_position(cg::geo_point_3(0.000,0.002*((double)pa_size+.1),0),cg::quaternion(cg::cpr(90, 0, 0))),
            ada::fill_data("BADA","A319"),                                                   
            s,
            0);


        _phys_aircrafts.emplace_back(boost::make_shared<aircraft::model>(nm::create_manager(node),ac_,s));
        _sys->registerBody(id,ac_->get_rigid_body());
        

        //_phys_aircrafts.back().shassis->visit_groups([=](aircraft::shassis_group_t & shassis_group)
        //{
        //    //if (to_be_opened)
        //    //    shassis_group.open(true);
        //    //else 
        //        if (!shassis_group.is_front)
        //                shassis_group.close(false);
        //});

        //addPhysicsData( id, positioned, pos, /*vel*/osg::Vec3(0.0,0.0,0.0), mass );
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",reinterpret_cast<uint32_t>(&*mt));
        mt->addChild( node );
        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;
    }

    void RigidUpdater::addVehicle(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {           
        // TODO FIXME И тут можно обдумать процесс управления всеми лодами сразу

        //osg::Node*  lod0 =  findFirstNode(node,"Lod0",findNodeVisitor::not_exact);
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        

        int id = _physicsNodes.size();
        phys::ray_cast_vehicle::info_ptr veh = _sys->createVehicle(lod3?lod3:node,id,mass);
        
        // FIXME
        if(lod3) 
            lod3->setNodeMask(0);

        //_sys->registerBody(id,phys::rigid_body_impl_ptr(veh)->get_body());
        
        _vehicles.emplace_back(veh);

        //addPhysicsData( id, positioned, pos, /*vel*/osg::Vec3(0.0,0.0,0.0), mass );
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",reinterpret_cast<uint32_t>(&*mt));
        mt->addChild( /*addGUIObject_v(node)*/node );
        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;

        _sys->setMatrix( id, osg::Matrix::translate(pos) );

        veh->set_steer(10);
    }

    void RigidUpdater::addVehicle2(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {           
        int id = _physicsNodes.size();
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
         
        nm::manager_ptr man = nm::create_manager(lod3?lod3:node);

        _phys_vehicles.emplace_back(vehicle::model::create(man,_sys));
        _sys->registerBody(id);  // FIXME Перевести внутрь модели 

        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",reinterpret_cast<uint32_t>(&*mt));
        mt->addChild( /*addGUIObject_v(node)*/node );
        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;

        _sys->setMatrix( id, osg::Matrix::translate(pos) );
        osg::Matrix mmm = _sys->getMatrix(id);
        _phys_vehicles.back()->set_state(vehicle::state_t(cg::geo_base_2()(from_osg_vector3(pos)), from_osg_quat(mmm.getRotate()).get_course(), 0.0f));

        //veh->set_steer(10);

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

    struct frame_timer
    {
        inline frame_timer(osgViewer::View* view,double& last_frame_time)
            : last_frame_time(last_frame_time)
            , current_time(0)
        {
           current_time = view->getFrameStamp()->getSimulationTime();
        }

        inline double diff()
        {
          return current_time - last_frame_time ; 
        }

        inline ~frame_timer()
        {
           last_frame_time  = current_time;
        }

    private:
        double  current_time;
        double& last_frame_time;
    };


    bool RigidUpdater::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = static_cast<osgViewer::View*>( &aa );

        frame_timer ftm (view,_last_frame_time);

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
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_O )
            {
// FIXME перенести в модель
#if 0
                _phys_aircrafts.back().get_chassis()->visit_groups([=](aircraft::shassis_group_t & shassis_group)
                {
                    //if (to_be_opened)
                    if (!shassis_group.is_front)
                        shassis_group.open(false);
                    //else
                    //  shassis_group.close(true);
                });
#endif
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_L )
            {
 // FIXME перенести в модель
#if 0
                _phys_aircrafts.back().get_chassis()->visit_groups([=](aircraft::shassis_group_t & shassis_group)
                {
                    //if (to_be_opened)
                    //    shassis_group.open(true);
                    //else
                    if (!shassis_group.is_front)
                        shassis_group.close(false);
                });
#endif
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_K )
            {
 // FIXME перенести в модель
#if 0
                {
                   _phys_aircrafts[0].check_wheel_brake();
                }
#endif                
            }

            break;
        case osgGA::GUIEventAdapter::FRAME:
            {
                double dt = _hr_timer.get_delta();
                double dt1 = ftm.diff();

                if (abs(dt-dt1)>0.1) 
                    OutputDebugString("Simulation time differ from real time more the 0.1 sec\n");

                if( _dbgDraw)
                    _dbgDraw->BeginDraw();

                _sys->update( dt );


                for(auto it = _phys_aircrafts.begin();it!=_phys_aircrafts.end();++it)
                {   
                     //high_res_timer        _hr_timer2;
     //               double dt2 = _hr_timer2.get_delta();
     //               std::stringstream cstr;
     //               cstr << std::setprecision(8) 
     //                   << "dt2:  "     << dt2 
     //                   <<"\n" ;
					//OutputDebugString(cstr.str().c_str());
                    
                    aircraft::int_control_ptr(*it)->update( dt );
                } 

                for(auto it = _phys_vehicles.begin();it!=_phys_vehicles.end();++it)
                {   
                          (*it)->update( view->getFrameStamp()->getSimulationTime()/*dt*/ );
                }   
                 

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
    
    void RigidUpdater::handleSelectObjectEvent(uint32_t id )
    {
         selected_obj_id_ = id;

         auto it_am = std::find_if(_phys_aircrafts.begin(),_phys_aircrafts.end(),[this](aircraft::info_ptr amp)->bool
         {
             if(amp->root()->object_id()==this->selected_obj_id_)
                 return true;

             return false;
         });

         if(it_am!=_phys_aircrafts.end())
         {
            auto traj = aircraft::int_control_ptr(*it_am)->get_trajectory().get();
            if (traj) _trajectory_drawer->set(*(traj));
         }
    }

    void RigidUpdater::handlePointEvent(std::vector<cg::point_3> const &simple_route)
    {   
        decart_position target_pos;
        target_pos.pos = simple_route.back();
        geo_position gp(target_pos, cg::geo_base_3(cg::geo_point_3(0.0,0.0,0)));
        
        if(selected_obj_id_)
        {
             
            auto it_am = std::find_if(_phys_aircrafts.begin(),_phys_aircrafts.end(),[this](aircraft::info_ptr amp)->bool
            {
                if(amp->root()->object_id()==this->selected_obj_id_)
                    return true;

                return false;
            });
            
            if(it_am!=_phys_aircrafts.end())
            {
                aircraft::info_ptr am=*it_am;
                decart_position cur_pos = aircraft::int_control_ptr(am)->get_local_position();
                target_pos.orien = cg::cpr(cg::polar_point_2(target_pos.pos - cur_pos.pos).course,0,0);

                if (!aircraft::int_control_ptr(am)->get_trajectory())
                {
                     aircraft::int_control_ptr(am)->set_trajectory( boost::make_shared<fms::trajectory>(cur_pos,target_pos,aircraft::model::min_radius(),aircraft::model::step()));
                }
                else
                {  
                    fms::trajectory_ptr main_ = aircraft::int_control_ptr(am)->get_trajectory();

                    decart_position begin_pos(cg::point_3(main_->kp_value(main_->length()),0)
                                             ,cg::cpr(main_->curs_value(main_->length()),0,0) );
            
                    target_pos.orien = cg::cpr(cg::polar_point_2(target_pos.pos - begin_pos.pos).course);
			
			        std::stringstream cstr;

                    cstr << std::setprecision(8) 
                        << "x:  "         << main_->kp_value(main_->length()).x
                        << "    y: "      << main_->kp_value(main_->length()).y
                        << "    curs :  " << main_->curs_value(main_->length()) 
                        << "    angle:  " << cg::angle(target_pos.pos,begin_pos.pos) 
                        << "    angle:  " << cg::polar_point_2(target_pos.pos - begin_pos.pos).course 
                        << "    delta curses: " << cg::norm360(target_pos.orien.get_course()) - cg::norm360(begin_pos.orien.get_course())
				        << "\n" ;

                    OutputDebugString(cstr.str().c_str());
       


                    fms::trajectory_ptr traj = boost::make_shared<fms::trajectory>( begin_pos,
                                                                                    target_pos,
                                                                                    aircraft::model::min_radius(),
                                                                                    aircraft::model::step());

                    main_->append(*traj.get());
                }
       
                // Подробная отрисовка
                _trajectory_drawer->set(*(aircraft::int_control_ptr(am)->get_trajectory().get()));

                auto vgp2 = fms::to_geo_points(*(aircraft::int_control_ptr(am)->get_trajectory().get()));
            }
            else
            {
                auto it_vh = std::find_if(_phys_vehicles.begin(),_phys_vehicles.end(),[this](vehicle::model_base_ptr vh)->bool
                {
                    if(vh->get_root()->object_id()==this->selected_obj_id_)
                        return true;

                    return false;
                });

                if(it_vh!=_phys_vehicles.end())
                    (*it_vh)->go_to_pos(gp.pos,90);
            }
            
        }

        // _trajectory_drawer->set(simple_route);

        //_phys_aircrafts[0].aircraft->go_to_pos(gp.pos ,gp.orien);
        
    }

}
