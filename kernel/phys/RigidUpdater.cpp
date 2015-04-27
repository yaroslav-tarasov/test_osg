#include "stdafx.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"

#include "utils/high_res_timer.h"
#include "BulletInterface.h"
#include "aircraft.h"                         // FIXME TODO don't need here 
#include "aircraft/aircraft_common.h"
#include "aircraft/aircraft_shassis_impl.h"   // And maybe this too
#include "nm/nodes_manager.h"
#include "ada/ada.h"
#include "bada/bada_import.h"
#include "aircraft/phys_aircraft.h"
#include "vehicle.h"

#include "common/simple_route.h"

#include "RigidUpdater.h"

#include "visitors/heil_visitor.h"

#include "object_creators.h"

// Моя фантазия
#include "objects/vehicle_fwd.h"
#include "objects/vehicle.h"



FIXME("kernel/systems.h")
#include "kernel/systems.h"

// FIXME
FIXME("Производящие функции либо в интерфейс,либо совсем отдельно")
namespace vehicle
{
	kernel::object_info_ptr create(kernel::system_ptr sys,nodes_management::manager_ptr nodes_manager,const std::string& model_name);
};

namespace aircraft
{
    kernel::object_info_ptr create(kernel::system_ptr sys,nodes_management::manager_ptr nodes_manager,phys::control_ptr        phys);
};

namespace
{
    struct krv_data
    {
        float x;
        float y; 
        float h;
        float fi;
        float fiw;
        float kr;
        float v;
        float w; 
        float vb;
        float tg;
        float time;
    };

    std::ostream &operator <<(std::ostream &os, const krv_data &kp) {
        using namespace std;

        for(size_t i = 0 ; i < sizeof(kp)/sizeof(float); ++i)
            os << *((float*)(&kp) + i*sizeof(float))  << "  ";
        return os;
    }

    struct value_getter
    {
        value_getter(std::string const& line)
        {
            boost::split(values_, line, boost::is_any_of(" \t="), boost::token_compress_on);
        }

        template <class T>
        T get(size_t index)
        {
            return boost::lexical_cast<T>(values_[index]);
        }

        bool valid()
        {
            return values_.size()>0;
        }

    private:
        std::vector<std::string> values_;
    };

    

    struct  krv_data_getter
    {
        std::vector<krv_data>    kd_;
        std::vector<cg::point_3> kp_;

        krv_data_getter(const std::string& file_name = std::string("log_AFL319.txt") )
        {
            std::ifstream ifs(file_name);

            int num =0;
            while (ifs.good())
            {
                char buf[0x400] = {};
                ifs.getline(buf, 0x400);

                std::string line = buf;
                value_getter items(line);
                krv_data kd;
                
                if(items.valid())
                {
                    kd.x = items.get<float>(1);
                    kd.y = items.get<float>(3); 
                    kd.h = items.get<float>(5);
                    //kd.fi = items.get<float>(7);
                    //kd.fiw = items.get<float>(9);
                    //kd.kr = items.get<float>(11);
                    //kd.v = items.get<float>(13);
                    //kd.w = items.get<float>(15); 
                    //kd.vb = items.get<float>(17);
                    //kd.tg = items.get<float>(19);
                    //kd.time = items.get<float>(21);

                    //kd_.push_back(kd);
                    kp_.push_back( cg::point_3(kd.x,kd.y,kd.h));
                }


                // std::cout << line;
            } 


        }
    };
};


namespace bi
{
	struct RigidUpdater::RigidUpdater_private
	{
		        RigidUpdater_private()
                    : _krv_data_getter("log_sochi_3.txt")
                {}
                RigidUpdater::phys_vehicles_t                          _vehicles;
                kernel::system_ptr                                     _msys;
				kernel::system_ptr                                     _vsys;
                kernel::system_ptr                                     _csys;
                kernel::msg_service                             msg_service_;
                krv_data_getter                             _krv_data_getter;
	};

    void create_auto_object(kernel::system_ptr sys, std::string class_name, std::string unique_name)
    {
        using namespace kernel;

        std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(sys)->object_classes() ;

        kernel::object_class_ptr class_ptr ;

        for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
        {
            if (class_name == (*it)->name())
                class_ptr = *it ;
            std::string n = (*it)->name();
        }

        auto obj = kernel::fake_objects_factory_ptr(sys)->create_object(class_ptr, unique_name); 

    }
    
    kernel::object_info_ptr create_object(kernel::system_ptr sys, std::string class_name, std::string unique_name)
    {
        using namespace kernel;

        std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(sys)->object_classes() ;

        kernel::object_class_ptr class_ptr ;

        for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
        {
            if (class_name == (*it)->name())
                class_ptr = *it ;
            std::string n = (*it)->name();
        }

        return kernel::fake_objects_factory_ptr(sys)->create_object(class_ptr, unique_name); 
    }

	RigidUpdater::RigidUpdater( osg::Group* root, on_collision_f on_collision ) 
		: _root        (root)
		, _on_collision(on_collision)
		, _dbgDraw     (nullptr)
		, _debug       (cfg().debug.debug_drawer)
		, _sys         (phys::create_phys_system())
		, _last_frame_time(0)
		, selected_obj_id_(0)
		, _d(boost::make_shared<RigidUpdater_private>())
        , _time_delta_mod_sys (0)
        , _time_delta_vis_sys (0)
        , _time_delta_ctrl_sys(0)
		, _trajectory_drawer2(new TrajectoryDrawer(root,TrajectoryDrawer::LINES))
	{
        
        using namespace kernel;
        _d->_msys = create_model_system(_d->msg_service_,"script should  be placed here");
        
        vis_sys_props props_;
        props_.base_point = ::get_base();

		_d->_vsys = create_visual_system(_d->msg_service_, props_);

        _d->_csys = create_ctrl_system(_d->msg_service_);
       
        create_auto_object(_d->_msys,"phys_sys","phys_sys");
        create_auto_object(_d->_msys,"airports_manager","aiports_manager");
        create_auto_object(_d->_msys,"ada","ada");
        create_auto_object(_d->_msys,"meteo_proxy","meteo_proxy");
        create_auto_object(_d->_msys,"airport","aiport_0");

        if(false)
        {
            std::string class_name = "aircraft";
            std::string unique_name = "aircraft_0";
            std::vector<object_class_ptr> const &classes = kernel::fake_objects_factory_ptr(_d->_msys)->object_classes() ;

            kernel::object_class_ptr class_ptr ;

            for (auto it = classes.begin(), end = classes.end(); it != end; ++it)
            {
                if (class_name == (*it)->name())
                    class_ptr = *it ;
                std::string n = (*it)->name();
            }

            auto obj = kernel::fake_objects_factory_ptr(_d->_msys)->create_object(class_ptr, unique_name); 
			nodes_management::manager_ptr manager = find_first_child<nodes_management::manager_ptr>(obj);
			manager->set_model(aircraft::get_model("A319"));
        }

        {
            cg::geo_point_3 apos(0.0,0.0005,0.0);
            aircraft::settings_t as;
            as.kind = "A319";
            geo_position agp(apos,quaternion(cpr(30,0,0)));
            auto obj_aircraft = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),as,agp);
        }

        {
            cg::geo_point_3 apos(0.0,0.0009,0.0);
            aircraft::settings_t as;
            as.kind = "A321";
            geo_position agp(apos,quaternion(cpr(60,0,0)));
            //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),as,agp);
        }

        {
            cg::geo_point_3 apos(0.0,0.0018,0.0);
            aircraft::settings_t as;
            as.kind = "B737";
            geo_position agp(apos,quaternion(cpr(60,0,0)));
            //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),as,agp);
        }

        {
            cg::geo_point_3 apos(0.0,0.0027,0.0);
            aircraft::settings_t as;
            as.kind = "B744";
            geo_position agp(apos,quaternion(cpr(60,0,0)));
            //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),as,agp);
        }

        {
            cg::geo_point_3 apos(0.0,0.0036,0.0);
            aircraft::settings_t as;
            as.kind = "B763";
            geo_position agp(apos,quaternion(cpr(60,0,0)));
            //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),as,agp);
        }

        {
            // cg::geo_point_3 apos(0.0,-0.0005/*0.0045*/,0.0);
            cg::point_3 vpos(350,650,0);
            decart_position target_pos(vpos,cg::quaternion(cg::cpr(30, 0, 0)));
            geo_position agp(target_pos, ::get_base());

            aircraft::settings_t as;
            as.kind = "AN26";//"A333";
            //geo_position agp(apos,quaternion(cpr(60,0,0)));
            //auto obj_aircraft2 = aircraft::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),as,agp);
        }

        vehicle::settings_t vs;
        vs.model = "buksir";//"niva_chevrolet";//
        
        //cg::point_3 vpos(330,750,00);
        //cg::point_3 vpos(572,032,0);
        //decart_position target_pos(vpos,cg::quaternion(cg::cpr(30, 0, 0)));
        //geo_position vgp(target_pos, ::get_base());
        
        cg::geo_point_3 vpos(0.0006,0.0009,0.0);
        geo_position vgp(vpos,quaternion(cpr(30,0,0)));
        
        auto obj_vehicle = vehicle::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),vs,vgp);


        //const kernel::object_collection  *  col = dynamic_cast<kernel::object_collection *>(_d->_csys.get());
        //auto vvv = find_object<vehicle::control_ptr>(col,"vehicle 0");
		//auto nm = find_first_child<nodes_management::manager_ptr>(vvv);
		//uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();
		//uint32_t vv_id = kernel::object_info_ptr(vvv)->object_id();

        // auto sr_obj = create_object(_d->_csys,"simple_route","simple_route_0");
        
        simple_route::settings_t srs;
        srs.speed = 6;
        auto sr_obj = simple_route::create(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_d->_csys).get()),srs,vgp.pos);

        _trajectory_drawer2->set(_d->_krv_data_getter.kp_);
        
    }

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
        phys::aircraft::control_ptr(_aircrafts.back())->apply_force(from_osg_vector3(vel));
    }


    void RigidUpdater::addUFO3(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {
        // TODO FIXME И тут можно обдумать процесс управления всеми лодами сразу
#if  0  // Under construction
        //osg::Node*  lod0 =  findFirstNode(node,"Lod0",findNodeVisitor::not_exact);
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        osg::Node*  root =  findFirstNode(node,"root",findNodeVisitor::not_exact);
        size_t object_id = 0;
        root->getUserValue("id",object_id);

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
        mt->setUserValue("id",object_id);
        mt->addChild( node );

        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;
#endif
    }

    void RigidUpdater::addUFO4(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {
        // TODO FIXME И тут можно обдумать процесс управления всеми лодами сразу

        //osg::Node*  lod0 =  findFirstNode(node,"Lod0",findNodeVisitor::not_exact);
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        osg::Node*  root =  findFirstNode(node,"root",findNodeVisitor::not_exact);
        size_t object_id = 0;
        root->getUserValue("id",object_id);

        int id = _physicsNodes.size();

        nm::manager_ptr man = nm::create_manager(_d->_msys,dict_t(),lod3);

        FIXME("А вот и засада с лодами")
        //aircraft::shassis_support_ptr s = boost::make_shared<aircraft::shassis_support_impl>(nm::create_manager(node));

        size_t pa_size = _model_aircrafts.size();

        _model_aircrafts.emplace_back(aircraft::create(_d->_msys,man,_sys) /*boost::make_shared<aircraft::model>(nm::create_manager(node),ac_,s)*/);
        _sys->registerBody(id);  // FIXME Перевести внутрь модели //_sys->registerBody(id,ac_->get_rigid_body());

        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",object_id);

        mt->addChild( node );

        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;
    }

    void RigidUpdater::addVehicle(osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {           
        // TODO FIXME И тут можно обдумать процесс управления всеми лодами сразу

        //osg::Node*  lod0 =  findFirstNode(node,"Lod0",findNodeVisitor::not_exact);
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        osg::Node*  root =  findFirstNode(node,"root",findNodeVisitor::not_exact);
        size_t object_id = 0;
        root->getUserValue("id",object_id);

        int id = _physicsNodes.size();
        phys::ray_cast_vehicle::info_ptr veh = _sys->createVehicle(node/*lod3?lod3:node*/,id,mass);
        
        FIXME("С начала вырубаем lod3 потом копируем модель, цирк ей богу")
        // FIXME
        //if(lod3) 
        //    lod3->setNodeMask(0);

        //_sys->registerBody(id,phys::rigid_body_impl_ptr(veh)->get_body());
        
        _d->_vehicles.emplace_back(veh);

        //addPhysicsData( id, positioned, pos, /*vel*/osg::Vec3(0.0,0.0,0.0), mass );
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",object_id);
        mt->addChild( /*addGUIObject_v(node)*/node );
        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;

        _sys->setMatrix( id, osg::Matrix::translate(pos) );

        //veh->set_steer(10);
    }

    void RigidUpdater::addVehicle2(const string& model_name,osg::Node* node,const osg::Vec3& pos, const osg::Vec3& vel, double mass)
    {           
        int id = _physicsNodes.size();
        osg::Node*  lod3 =  findFirstNode(node,"Lod3",findNodeVisitor::not_exact);
        osg::Node*  root =  findFirstNode(node,"root",findNodeVisitor::not_exact); 
        size_t object_id = 0;
        root->getUserValue("id",object_id);

#ifdef  OSG_NODE_IMPL
        lod3?lod3->setNodeMask(0):0;
        nm::manager_ptr man = nm::create_manager(_d->_msys,dict_t(),node);
        //lod3?lod3->setNodeMask(0xffffffff):0;
#else
        nm::manager_ptr man = nm::create_manager(_d->_msys,dict_t(),nullptr/*lod3?lod3:node*/);
        object_id  = man->get_node(0)->object_id();
#endif

        FIXME("Костыль")
        man->set_model(model_name);

        _phys_vehicles.push_back(vehicle::create(_d->_msys,man,model_name));
        _sys->registerBody(id);  // FIXME Перевести внутрь модели 
        //man->set_model(model_name);

        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",object_id);
        
        mt->addChild( /*addGUIObject_v(node)*/node );


        _root->addChild( mt.get() );

        _physicsNodes[id] = mt;

        _sys->setMatrix( id, osg::Matrix::translate(pos) );
        osg::Matrix mmm = _sys->getMatrix(id);
        _phys_vehicles.back()->set_state_debug(vehicle::state_t(cg::geo_base_2()(from_osg_vector3(pos)), from_osg_quat(mmm.getRotate()).get_course(), 0.0f));

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

                auto vvv = kernel::find_object<aircraft::control_ptr>(kernel::object_collection_ptr(_d->_csys).get(),"aircraft 0");
                if(vvv)
                {
                      aircraft::aircraft_ipo_control_ptr(vvv)->set_malfunction(aircraft::MF_FIRE_ON_BOARD,true);   
                }

            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_M /*&& (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)*/)
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
                auto vvv = kernel::find_object<vehicle::control_ptr>(kernel::object_collection_ptr(_d->_csys).get(),"vehicle 0");
                if(vvv)
                {
                    vvv->attach_tow();      
                }
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_N /*&& (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)*/)
            {
                auto vvv = kernel::find_object<vehicle::control_ptr>(kernel::object_collection_ptr(_d->_csys).get(),"vehicle 0");
                if(vvv)
                {
                    vvv->detach_tow();
                }
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_K )
            {
                 auto vvv = kernel::find_object<vehicle::control_ptr>(kernel::object_collection_ptr(_d->_csys).get(),"vehicle 0");
                 auto sr_obj = kernel::find_object<simple_route::control_ptr>(kernel::object_collection_ptr(_d->_csys).get(),"simple_route 0");
                 if(sr_obj)
                 {
                    sr_obj->set_speed(5);
                    vvv->follow_route("simple_route 0");
                 }
                 
            }
            else if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_B )
            {
                auto vvv = kernel::find_object<vehicle::control_ptr>(kernel::object_collection_ptr(_d->_csys).get(),"vehicle 0");
                vvv->set_brake(0.35);
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

                //_sys->update( dt );

                const double dt_sys = dt; 

                if(_time_delta_ctrl_sys >= cfg().model_params.csys_step)
                {
                    _d->_csys->update(view->getFrameStamp()->getSimulationTime());
                    _time_delta_ctrl_sys = dt_sys;
                }
                else
                    _time_delta_ctrl_sys += dt_sys;


                // Физику обновляем через моделирующую 
                if(_time_delta_mod_sys >= cfg().model_params.msys_step)
                {
                    _d->_msys->update(view->getFrameStamp()->getSimulationTime());
                    _time_delta_mod_sys = dt_sys;
                }
                else
                    _time_delta_mod_sys += dt_sys;

                LOG_ODS_MSG("Update-------------------------" << "  dt=  " << dt << "--------------------------------\n");
                


                //if(_time_delta_vis_sys >= cfg().model_params.vsys_step)
                {
                    _d->_vsys->update(view->getFrameStamp()->getSimulationTime());
                    _time_delta_vis_sys = dt_sys;
                }
                //else
                //    _time_delta_vis_sys += dt_sys;
                
                
                

                for(auto it = _model_aircrafts.begin();it!=_model_aircrafts.end();++it)
                {   
                     //high_res_timer        _hr_timer2;
     //               double dt2 = _hr_timer2.get_delta();
     //               std::stringstream cstr;
     //               cstr << std::setprecision(8) 
     //                   << "dt2:  "     << dt2 
     //                   <<"\n" ;
					//OutputDebugString(cstr.str().c_str());
                    
                    aircraft::int_control_ptr(*it)->update( view->getFrameStamp()->getSimulationTime()/*dt*/ );
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




    //void RigidUpdater::createNodeHierarchy(osg::Node* node)
    //{
    //      std::ofstream filelogic(std::string("test") + ".stbin", std::ios_base::binary);
    //      
    //      heilVisitor  hv(filelogic);
    //      hv.apply(*node);
    //}


    void RigidUpdater::handleSelectObjectEvent(uint32_t id )
    {
         selected_obj_id_ = id;

         auto it_am = std::find_if(_model_aircrafts.begin(),_model_aircrafts.end(),[this](aircraft::info_ptr amp)->bool
         {
             if(amp->root() && amp->root()->object_id()==this->selected_obj_id_)
                 return true;

             return false;
         });

         if(it_am!=_model_aircrafts.end())
         {
            auto traj = aircraft::int_control_ptr(*it_am)->get_trajectory();
            if (traj) _trajectory_drawer->set(traj);
         } 

         bool a_or_v = false;

         const kernel::object_collection  *  col = dynamic_cast<kernel::object_collection *>(_d->_csys.get());

         kernel::visit_objects<vehicle::control_ptr>(col,[this,&a_or_v](vehicle::control_ptr a)->bool
         {
             auto nm = kernel::find_first_child<nodes_management::manager_ptr>(a);
             uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();

             if( nm_id == this->selected_obj_id_)
             {
                 selected_object_type_signal_(VEHICLE_TYPE);
                 a_or_v = true;
                 return false;
             }
             return true;
         });

         kernel::visit_objects<aircraft::control_ptr>(col,[this,&a_or_v](aircraft::control_ptr a)->bool
         {
             auto nm = kernel::find_first_child<nodes_management::manager_ptr>(a);
             uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();

             if( nm_id == this->selected_obj_id_)
             {
                 selected_object_type_signal_(AIRCRAFT_TYPE);
                 a_or_v = true;
                 return false;
             }
             return true;
         });

         if(!a_or_v)
            selected_object_type_signal_(NONE_TYPE);
         
    }

    FIXME(Нужен ли полный путь? Не думаю.)
    void RigidUpdater::handlePointEvent(std::vector<cg::point_3> const &cartesian_simple_route)
    {   
        decart_position target_pos;
        target_pos.pos = cartesian_simple_route.back();
        geo_position gp(target_pos, ::get_base());
        
        if(selected_obj_id_)
        {
             
            auto it_am = std::find_if(_model_aircrafts.begin(),_model_aircrafts.end(),[this](aircraft::info_ptr amp)->bool
            {
                if(amp->root() && amp->root()->object_id()==this->selected_obj_id_)
                    return true;

                return false;
            });
            
            if(it_am!=_model_aircrafts.end())
            {
                aircraft::info_ptr am=*it_am;
                decart_position cur_pos = aircraft::int_control_ptr(am)->get_local_position();
                target_pos.orien = cg::cpr(cg::polar_point_2(target_pos.pos - cur_pos.pos).course,0,0);

                if (!aircraft::int_control_ptr(am)->get_trajectory())
                {
                     aircraft::int_control_ptr(am)->set_trajectory( fms::trajectory::create(cur_pos,target_pos,aircraft::min_radius(),aircraft::step()));
                }
                else
                {  
                    fms::trajectory_ptr main_ = aircraft::int_control_ptr(am)->get_trajectory();

                    decart_position begin_pos(cg::point_3(main_->kp_value(main_->length()),0)
                                             ,cg::cpr(main_->curs_value(main_->length()),0,0) );
            
                    target_pos.orien = cg::cpr(cg::polar_point_2(target_pos.pos - begin_pos.pos).course);
			
                    fms::trajectory_ptr traj = fms::trajectory::create ( begin_pos,
                                                                         target_pos,
                                                                         aircraft::min_radius(),
                                                                         aircraft::step());

                    main_->append(traj);
                }
       
                // Подробная отрисовка
                _trajectory_drawer->set(aircraft::int_control_ptr(am)->get_trajectory());

                auto vgp2 = fms::to_geo_points(*(aircraft::int_control_ptr(am)->get_trajectory().get()));
            }
            else
            {
                auto it_vh = std::find_if(_phys_vehicles.begin(),_phys_vehicles.end(),[this](vehicle::model_base_ptr vh)->bool
                {
                    if(vh->get_root() && vh->get_root()->object_id()==this->selected_obj_id_)
                        return true;

                    return false;
                });

                if(it_vh!=_phys_vehicles.end())
                    (*it_vh)->go_to_pos(gp.pos,90);
            }
            
            const kernel::object_collection  *  col = dynamic_cast<kernel::object_collection *>(_d->_csys.get());
#if 0  // Рабочий код ездим           
            kernel::visit_objects<vehicle::control_ptr>(col,[this,&gp](vehicle::control_ptr a)->bool
            {
				auto nm = kernel::find_first_child<nodes_management::manager_ptr>(a);
				uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();

                if( nm_id == this->selected_obj_id_)
                {
                     a->goto_pos(gp.pos,90);
                     return false;
                }
                return true;
            });
#endif

            kernel::visit_objects<vehicle::control_ptr>(col,[this,&gp](vehicle::control_ptr a)->bool
            {
                auto nm = kernel::find_first_child<nodes_management::manager_ptr>(a);
                uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();

                if( nm_id == this->selected_obj_id_)
                {
                    auto sr_obj = kernel::find_first_object<simple_route::control_ptr>(kernel::object_collection_ptr(_d->_csys).get());
                    sr_obj->add_point(gp.pos);
                    return false;
                }
                return true;
            });

            FIXME(Гиде контроль?)
            const kernel::object_collection  *  mcol = dynamic_cast<kernel::object_collection *>(_d->_msys.get());
            
            kernel::visit_objects<aircraft::control_ptr>(mcol,[this,&target_pos](aircraft::control_ptr a)->bool
            {
                auto nm = kernel::find_first_child<nodes_management::manager_ptr>(a);
                uint32_t nm_id = kernel::object_info_ptr(nm)->object_id();

                if( nm_id == this->selected_obj_id_)
                {
                    aircraft::info_ptr am= aircraft::info_ptr(a);
                    
                    decart_position cur_pos = aircraft::int_control_ptr(am)->get_local_position();
                    target_pos.orien = cg::cpr(cg::polar_point_2(target_pos.pos - cur_pos.pos).course,0,0);

                    if (!aircraft::int_control_ptr(am)->get_trajectory())
                    {
                        aircraft::int_control_ptr(am)->set_trajectory( fms::trajectory::create(cur_pos,target_pos,aircraft::min_radius(),aircraft::step()));
                    }
                    else
                    {  
                        fms::trajectory_ptr main_ = aircraft::int_control_ptr(am)->get_trajectory();

                        decart_position begin_pos(cg::point_3(main_->kp_value(main_->length()),0)
                            ,cg::cpr(main_->curs_value(main_->length()),0,0) );

                        target_pos.orien = cg::cpr(cg::polar_point_2(target_pos.pos - begin_pos.pos).course);

                        fms::trajectory_ptr traj = fms::trajectory::create( begin_pos,
                                                                            target_pos,
                                                                            aircraft::min_radius(),
                                                                            aircraft::step());

                        main_->append(traj);
                    }

                    // Подробная отрисовка
                    _trajectory_drawer->set(aircraft::int_control_ptr(am)->get_trajectory());

                    auto vgp2 = fms::to_geo_points(*(aircraft::int_control_ptr(am)->get_trajectory().get()));


                    return false;
                }
                return true;
            });

        }






        // _trajectory_drawer->set(simple_route);

        //_phys_aircrafts[0].aircraft->go_to_pos(gp.pos ,gp.orien);
        
    }

}
