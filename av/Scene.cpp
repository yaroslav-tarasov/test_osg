#include "stdafx.h"

#include "high_res_timer.h"
#include "phys/BulletInterface.h"
#include "phys/RigidUpdater.h"

#include <osg/GLObjects>

//Degree precision versus length
//
//          decimal                                                                               N/S or E/W    E/W at    E/W at       E/W at
//          degrees           DMS                 qualitative scale that can be identified        at equator    23N/S      45N/S	     67N/S
//    0	    1.0	        1° 00′ 0″	        country or large region	                            111.32 km	102.47 km	78.71 km	43.496 km
//    1	    0.1	        0° 06′ 0″	        large city or district	                            11.132 km	10.247 km	7.871 km	4.3496 km
//    2 	0.01	    0° 00′ 36″	        town or village	                                    1.1132 km	1.0247 km	787.1 m	    434.96 m
//    3	    0.001	    0° 00′ 3.6″	        neighbourhood, street	                            111.32 m	102.47 m	78.71 m	    43.496 m
//    4 	0.0001	    0° 00′ 0.36″	    individual street, land parcel	                    11.132 m	10.247 m	7.871 m	    4.3496 m
//    5	    0.00001	    0° 00′ 0.036″	    individual trees	                                1.1132 m	1.0247 m	787.1 mm	434.96 mm
//    6	    0.000001	0° 00′ 0.0036″	    individual humans	                                111.32 mm	102.47 mm	78.71 mm	43.496 mm
//    7	    0.0000001	0° 00′ 0.00036″	    practical limit of commercial surveying	            11.132 mm	10.247 mm	7.871 mm	4.3496 mm
//    8	    0.00000001	0° 00′ 0.000036″	specialized surveying (e.g. tectonic plate mapping)	1.1132 mm	1.0247 mm	787.1 µm	434.96 µm
      

#include "Scene.h"
#include "creators.h"
#include "Ephemeris.h"
#include "sm/ShadowedScene.h"
#include "sm/ShadowMap.h"
#include "sm/ViewDependentShadowMap.h"
#include "Terrain.h"

#include "pickhandler.h"

#include "application/panels/vis_settings_panel.h"

namespace gui 
{ 
    osgGA::GUIEventHandler*  createCEGUI(osg::Group* root, std::function<void()> init_gui_handler);
    void    releaseCEGUI();
}

using namespace avScene;

std::vector<osg::ref_ptr<osg::Node>> _lamps;

namespace {

    inline osg::Vec3f lights_offset(std::string name)
    {
        if (name == "sheremetyevo")
            return osg::Vec3f(18, 17, .2f);
        else if (name == "adler")
            return osg::Vec3f(0 , 0 , .2f);

        return osg::Vec3f();
    }

    struct value_getter
    {
        value_getter(std::string const& line)
        {
            boost::split(values_, line, boost::is_any_of(" \t"), boost::token_compress_on);
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

template<typename S> 
inline osg::Vec3f polar_point_2(S range, S course )
{
    return osg::Vec3( (S)range * sin(cg::grad2rad(course)),
        (S)range * cos(cg::grad2rad(course)),0);
}

// typedef osg::ref_ptr<osg::Group> navaid_group_node_ptr;
typedef osg::ref_ptr<osgSim::LightPointNode> navaid_group_node_ptr;

osg::Geode* CreateLight (const osg::Vec4& fcolor,const std::string& name,osg::NodeCallback* callback)
{
    static std::map<osg::Vec4,osg::Geode*> colors;
    if(colors.find ( fcolor ) == colors.end())
    {
        osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
        hints->setDetailRatio(0.1f); 
        hints->setTargetNumFaces(2);
        osg::ref_ptr<osg::ShapeDrawable> shp = new osg::ShapeDrawable(nullptr,hints);
        shp->setShape( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.3f) );
        osg::Geode* light = new osg::Geode;
        light->addDrawable( shp.get() );
        dynamic_cast<osg::ShapeDrawable *>(light->getDrawable(0))->setColor( fcolor );
        light->setUpdateCallback(callback);
        light->setName(name);
        light->setCullingActive(false);
        colors [fcolor] = light;
    }



#if 0  // Перенесено см ниже
    const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED| osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF;
    osg::StateSet* ss = light->getOrCreateStateSet();
    ss->setAttribute(new osg::Program(),value);
    ss->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
    ss->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    // ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    ss->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");
    // ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF |  osg::StateAttribute::PROTECTED);
#endif    

    return  colors [fcolor];
};

void fill_navids(std::string file, std::vector<osg::ref_ptr<osg::Node>>& cur_lamps, osg::Group* parent, osg::Vec3f const& offset)
{
    //     if (!boost::filesystem::is_regular_file(file))
    //         LogWarn("No lights for airport found: " << file.string());

    std::ifstream ifs(file);

    for (auto it = cur_lamps.begin(); it != cur_lamps.end(); ++it)
        parent->removeChild(it->get());
    cur_lamps.clear();

    navaid_group_node_ptr navid_node_ptr = nullptr;

    bool group_ready = false;

    while (ifs.good())
    {
        char buf[0x400] = {};
        ifs.getline(buf, 0x400);

        std::string line = buf;
        boost::trim(line);

        // skip comments

        if (line.size()==0) continue;

        if (boost::starts_with(line, "//"))
        {
            boost::erase_all(line," ");
            if (boost::starts_with(line, "//#"))
            {
                boost::trim_if(line, boost::is_any_of("/#"));
                //navid_node_ptr.reset(static_cast<victory::navaid_group_node *>(fabric->create(victory::node::NT_NavAidGroup).get()));
                navid_node_ptr.release();
                navid_node_ptr = new osgSim::LightPointNode;// new osg::Group();
                navid_node_ptr->setName(line);
                group_ready = true;
            }
            else
                continue;
        }

        value_getter items(line);

        if (items.get<std::string>(0) == "FireLine:" || items.get<std::string>(0) == "FireLineHa:")
        {
            size_t ofs = items.get<std::string>(0) == "FireLineHa:" ? 3 : 0;

            osg::Vec3f pos (items.get<float>(ofs + 2), items.get<float>(ofs + 4), items.get<float>(ofs + 3));
            osg::Vec3f dir (polar_point_2(1.f, items.get<float>(ofs + 5)));
            float      len   = items.get<float>(ofs + 6);
            float      step  = items.get<float>(ofs + 7);
            osg::Vec4f clr (items.get<float>(ofs + 8), items.get<float>(ofs + 9), items.get<float>(ofs + 10),1.0f); 

            size_t count = 0;
            if (!cg::eq_zero(step) && !cg::eq_zero(len))
            {
                count = size_t(cg::ceil(len / step));

                if (step > 2)
                    ++count;
            }

            for (size_t i = 0; i < count; ++i)
            {
                osg::Vec3f p = pos + dir * step * i  + offset;

                //victory::navaid_group_node::LightData lamp = {p, clr, .1,40000,/*.01f, 4000.f,*/ cg::range_2f(), cg::range_2f(), 1, 0, 0};
                //navid_group->AddLight(lamp);
                
                osgSim::LightPoint pnt;

                pnt._position.set(p.x(),p.y(),p.z());
                pnt._color = clr;
                pnt._radius = 0.3f;
                navid_node_ptr->addLightPoint(pnt);

                //osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();

                //pat->addChild(CreateLight(clr,std::string("light"),nullptr));
                //pat->setPosition(osg::Vec3f(p.x(),p.y(),p.z()));
                //navid_node_ptr->addChild(pat);

                //const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED| osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF;
                //osg::StateSet* ss = pat->getOrCreateStateSet();
                //ss->setAttribute(new osg::Program(),value);
                //ss->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
                //ss->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
                //ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE );
                //ss->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");


            }
        }
        
        if(navid_node_ptr && group_ready)
        {
            cur_lamps.push_back(navid_node_ptr);
            parent->addChild(navid_node_ptr);
            group_ready = false;
        }

    }


    //cur_lamps.push_back(navid_group);
    //parent->add(navid_group);
}

}

namespace avGUI {

    template <class T>
    class GUIEventHandlerImpl : public osgGA::GUIEventHandler
    {
    public:
        GUIEventHandlerImpl( T * object, bool (T::*func)( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv ) ) 
            : _object(object)
            , _func(func)
        {
        }

        virtual bool handle( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv )
        {
            return (_object->*_func)(ea, aa, obj, nv) || osgGA::GUIEventHandler::handle(ea, aa, obj, nv);
        }

    private:
        T * _object;
        bool (T::*_func)( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv );
    };

    template<class T>
    inline GUIEventHandlerImpl<T> * makeGUIEventHandlerImpl( T * object, bool (T::*func)( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv ) )
    {
        return new GUIEventHandlerImpl<T>(object, func);
    }

} // namespace avGUI


osg::ref_ptr<Scene>	 Scene::_scenePtr;

std::string          Scene::zone_to_reload_;

//////////////////////////////////////////////////////////////////////////
bool Scene::Create( osg::ArgumentParser& cArgs, osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr )
{
    _scenePtr = new Scene();
    if ( _scenePtr->Initialize( cArgs, cTraitsPtr, cTraitsPtr.valid() ? cTraitsPtr->width : 0, cTraitsPtr.valid() ? cTraitsPtr->height : 0 ) == false )
    {
        OSG_FATAL << "Failed to initialize scene" ;
        _scenePtr = NULL;
    }

    return _scenePtr.valid();
}
//////////////////////////////////////////////////////////////////////////
void Scene::Release()
{
    if ( _scenePtr.valid() )
    {

        // Release smart pointer with cross references
        _scenePtr->_viewerPtr	= NULL;
        
        gui::releaseCEGUI();

        _scenePtr->_rigidUpdater->stopSession();

        // Release scene
        _scenePtr = NULL;

        creators::releaseObjectCache();
    }



    FIXME( Не в сцене этому место задвигать дальше надо)
    osg::discardAllGLObjects(0);
}

//////////////////////////////////////////////////////////////////////////
Scene* Scene::GetInstance()
{
    return _scenePtr.get();
}



//////////////////////////////////////////////////////////////////////////
Scene::Scene()
{
    // Common nodes for scene etc.
    _commonNode = new Group();

    osg::StateSet * pCommonStateSet = _commonNode->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");

    addChild(_commonNode.get());

    // Add backface culling to the whole bunch
    //osg::StateSet * pSS = getOrCreateStateSet();
    //pSS->setNestRenderBins(false);
    //pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    //// disable alpha writes for whole bunch
    //pSS->setAttribute(new osg::ColorMask(true, true, true, false)); 

}

//////////////////////////////////////////////////////////////////////////
Scene::~Scene()
{
}

//////////////////////////////////////////////////////////////////////////
bool Scene::Initialize( osg::ArgumentParser& cArgs, osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr, int nWidth, int nHeight)
{
    osg::StateSet* pGlobalStateSet = getOrCreateStateSet();
    osg::StateSet* pCommonStateSet = getCommonNode()->getOrCreateStateSet();
    
    const int nAntialiasing = 8;
    

    if ( cTraitsPtr.valid())
    {
        cTraitsPtr->samples = nAntialiasing;
    }
    else 
    {
        osg::DisplaySettings::instance()->setNumMultiSamples( nAntialiasing );
    }

    // Create viewer and 
    _viewerPtr = new osgViewer::Viewer(cArgs);
     
    _viewerPtr->apply(new osgViewer::SingleScreen(1));

    // Set up camera
    if ( cTraitsPtr.valid() == true )
    {
        osg::ref_ptr<osg::GraphicsContext> cGraphicsContextPtr = osg::GraphicsContext::createGraphicsContext( cTraitsPtr.get() );

        _viewerPtr->getCamera()->setGraphicsContext(cGraphicsContextPtr.get());
        _viewerPtr->getCamera()->setViewport(new osg::Viewport(0, 0, nWidth, nHeight));
    } 
    else if (getenv("OSG_SCREEN") == NULL)
    {
        //_viewerPtr->setUpViewInWindow(20, 20, 820, 620);
    }

    _viewerPtr->getCamera()->setClearColor(osg::Vec4(0.f, 0.f, 0.f, 1.f));
    //FIXME TODO //setProjectionMatrixFromConfig();

    _viewerPtr->getCamera()->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);    
    // _viewerPtr->getCamera()->setSmallFeatureCullingPixelSize(10.0F);

    _viewerPtr->setSceneData( this );
    // _viewerPtr->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // TODO: enabled this for instructor tab, need implement special setting
    //_viewerPtr->setReleaseContextAtEndOfFrameHint(false); 

    // disable ESC key
    _viewerPtr->setKeyEventSetsDone(0);

    //
    // Set up the camera manipulators.
    //

    // Use a default camera manipulator
    osgGA::FirstPersonManipulator* manip = new osgGA::FirstPersonManipulator;
     //new osgGA::DriveManipulator()
     //new osgGA::TerrainManipulator() 
     //new osgGA::OrbitManipulator() 
     //new osgGA::FirstPersonManipulator() 
     //new osgGA::SphericalManipulator() 

    manip->setAcceleration(0);
    manip->setMaxVelocity(1);
    manip->setWheelMovement(10,true);
    //manip->setWheelMovement(0.001,true);
    _viewerPtr->setCameraManipulator(manip);
    //manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
    //manip->home(0);

    //
    // Add event handlers to the viewer
    //
    _pickHandler = new PickHandler();
    
    if(true) _viewerPtr->addEventHandler( 
        gui::createCEGUI( _commonNode, [this]()
        {
            if (!this->_vis_settings_panel )
            {
                app::zones_t zones_;
                zones_.push_back(std::make_pair(0,std::wstring(L"Пустая сцена")));
                zones_.push_back(std::make_pair(1,std::wstring(L"Шереметьево")));
                zones_.push_back(std::make_pair(2,std::wstring(L"Сочи")));
                this->_vis_settings_panel = app::create_vis_settings_panel( zones_ );
                this->_vis_settings_panel->subscribe_zone_changed(boost::bind(&Scene::onZoneChanged,this,_1));
                this->_vis_settings_panel->subscribe_exit_app    ([=]() { exit(0); });
                this->_vis_settings_panel->set_visible(false);
            }
        } )
    );
     
    _viewerPtr->addEventHandler( new osgViewer::WindowSizeHandler );
    _viewerPtr->addEventHandler( new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()) );
    _viewerPtr->addEventHandler( new osgViewer::StatsHandler );
    _viewerPtr->addEventHandler( _pickHandler );    
    _viewerPtr->addEventHandler( avGUI::makeGUIEventHandlerImpl(this, &Scene::onEvent));
    _viewerPtr->realize();
    addChild(_pickHandler->getOrCreateSelectionBox()); 
    


    //
    // Initialize particle engine
    // 

    spark::init();
    
    //
    // And some fire
    //

    //spark::spark_pair_t sp =   spark::create(spark::FIRE);
    //spark::spark_pair_t sp2 =  spark::create(spark::EXPLOSION);

    //osg::MatrixTransform* posed = new osg::MatrixTransform(osg::Matrix::translate(osg::Vec3(400.0,400.0,50.0)));
    //posed->addChild(sp.first);
    //posed->addChild(sp2.first);
    //_viewerPtr->addEventHandler(sp.second);
    //_viewerPtr->addEventHandler(sp2.second);
    //addChild(posed);

    //
    // Create terrain / shadowed scene
    //

    createTerrainRoot();
    
    std::string scene_name("minsk"); // "empty","adler" ,"sheremetyevo"

    _terrainNode =  new avTerrain::Terrain (_terrainRoot);
    _terrainNode->create(scene_name);

#if 1
    fill_navids(
        scene_name + ".txt", 
        _lamps, 
        this, 
        lights_offset(scene_name) ); 
#endif

    osg::Node* ct =  findFirstNode(_terrainNode,"camera_tower");

    if(ct) 
    {
        osg::Vec3 c = ct->asTransform()->asMatrixTransform()->getMatrix().getTrans();
        manip->setHomePosition(c, osg::Vec3(0,1,0), osg::Z_AXIS);
        manip->home(0);
    }
	else
	{
	    manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
        manip->home(0);
	}

    //
    // Init physic updater
    //

    createObjects();
    
    conn_holder_ << _pickHandler->subscribe_choosed_point(boost::bind(&bi::RigidUpdater::handlePointEvent, _rigidUpdater.get(), _1));
    conn_holder_ << _pickHandler->subscribe_selected_node(boost::bind(&bi::RigidUpdater::handleSelectObjectEvent, _rigidUpdater.get(), _1));
   
    conn_holder_ << _rigidUpdater->subscribe_selected_object_type(boost::bind(&PickHandler::handleSelectObjectEvent, _pickHandler.get(), _1));
    _rigidUpdater->setTrajectoryDrawer(new TrajectoryDrawer(this,TrajectoryDrawer::LINES));
    

    //
    // Create ephemeris
    //                                                                       
    _ephemerisNode = new avSky::Ephemeris( this,
                                           _terrainNode.get(),
                                          [=](float illum){ if(_st!=0) _st->setNightMode(illum < .35);  } //  FIXME magic night value    
    );  



    //
    //  Get or create sunlight
    //
    
    if( _ephemerisNode->getSunLightSource())
    {
        _ls =_ephemerisNode->getSunLightSource();  
        if(_terrainRoot) _terrainRoot->addChild(_ls.get());
    }
    else
    {
        _ls = new osg::LightSource;
        _ls->getLight()->setLightNum(0);
        if(_terrainRoot) _terrainRoot->addChild(_ls.get());
        _ephemerisNode->setSunLightSource(_ls);
    }

    addChild( _ephemerisNode.get() );

    _viewerPtr->addEventHandler(_ephemerisNode->getEventHandler());
    


    //
    // Create weather
    //
    _weatherNode =  new osg::Group;
    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;		
    precipitationEffect->snow(0.0);
    precipitationEffect->setWind(osg::Vec3(0.2f,0.2f,0.2f));

    _weatherNode->addChild( precipitationEffect.get() );
    addChild( _weatherNode.get() );

    osg::MatrixTransform* transform = new osg::MatrixTransform;

    transform->setDataVariance(osg::Object::STATIC);
    transform->setMatrix(osg::Matrix::scale(0.1,0.1,0.1));

    return true;
}


void  Scene::createTerrainRoot()
{

#if defined(TEST_SHADOWS_FROM_OSG)

    const int fbo_tex_size = 1024*4;

    _st = new avShadow::ViewDependentShadowMap;

     _terrainRoot
        = new avShadow::ShadowedScene(_st.get());  

    avShadow::ShadowSettings* settings = dynamic_cast<avShadow::ShadowedScene*>(_terrainRoot.get())->getShadowSettings();

    settings->setShadowMapProjectionHint(avShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);   //ORTHOGRAPHIC_SHADOW_MAP
    settings->setBaseShadowTextureUnit(BASE_SHADOW_TEXTURE_UNIT);
    settings->setMinimumShadowMapNearFarRatio(.5);
    //settings->setNumShadowMapsPerLight(/*numShadowMaps*/2);
    //settings->setMultipleShadowMapHint(avShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setMultipleShadowMapHint(avShadow::ShadowSettings::CASCADED);
    settings->setTextureSize(osg::Vec2s(fbo_tex_size,fbo_tex_size));
    //settings->setLightNum(2);
    settings->setMaximumShadowMapDistance(1500);
    settings->setShaderHint(avShadow::ShadowSettings::NO_SHADERS);

#else
    _terrainRoot = new osg::Group;
#endif     

     addChild(_terrainRoot);
}

void Scene::createObjects()
{
 
    _rigidUpdater = new bi::RigidUpdater( _terrainRoot->asGroup() 
        ,[&](osg::MatrixTransform* mt){ 
            if(!findFirstNode(mt,"fire"))
            {
                spark::spark_pair_t sp3 =  spark::create(spark::FIRE,mt);
                sp3.first->setName("fire");
                mt->addChild(sp3.first);
            }
    }
    );

    //auto heli = creators::applyBM(creators::loadHelicopter(),"mi_8",true);
    //_terrainRoot->addChild(heli);


    if(_rigidUpdater.valid())
        _rigidUpdater->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );

    const std::string name = "a_319";


	auto obj = creators::createObject(name,true);


#if 0
	if(_rigidUpdater.valid())
		_rigidUpdater->addPhysicsAirplane( obj,
		osg::Vec3(0,0,0), osg::Vec3(0,60,0), 800.0f );


	if(_rigidUpdater.valid())
		_rigidUpdater->addUFO( obj,
		osg::Vec3(100,100,0), osg::Vec3(0,0,0), 165.0f );

#endif

#if 0 
	if(_rigidUpdater.valid())
		_rigidUpdater->addUFO2( obj,
		osg::Vec3(-100,-100,0), osg::Vec3(0,100000,0), 1650.0f );   // force 
#endif

#if 0 
    if(_rigidUpdater.valid())
        _rigidUpdater->addUFO2( obj,
        osg::Vec3(150,-150,00), osg::Vec3(0,30000,0), 1650.0f );    // force
#endif

    //if(_rigidUpdater.valid())
    //    _rigidUpdater->addUFO3( obj,
    //    osg::Vec3(-100,-100,0), osg::Vec3(0,0,0), 1650.0f );   // force 

#if 0 
    if(_rigidUpdater.valid())
        _rigidUpdater->addUFO4( creators::createObject(name,true),
        osg::Vec3(-50,-50,10), osg::Vec3(0,0,0), 1650.0f );   // force 

    if(_rigidUpdater.valid())
        _rigidUpdater->addUFO4( creators::createObject(name,true),
        osg::Vec3(-50,-50,10), osg::Vec3(0,0,0), 1650.0f );   // force 
    
    if(_rigidUpdater.valid())
        _rigidUpdater->addUFO4( creators::createObject(name,true),
        osg::Vec3(-50,-50,10), osg::Vec3(0,0,0), 1650.0f );   // force 
#endif

    const bool add_planes = false;

    if (add_planes)
    {

        osg::Node* p_copy = creators::applyBM(creators::loadAirplane(name),name,true);
        // auto p_copy = creators::loadAirplane(name); // А если без BM еще кадров 15-20 ??? Чет не вижу
        
        float rot_angle = -90.f;
        if(dynamic_cast<osg::LOD*>(p_copy))
            rot_angle = 0;  

        for ( unsigned int i=0; i<10; ++i )
        {
            for ( unsigned int j=0; j<10; ++j )
            {
                if(_rigidUpdater.valid())
                    _rigidUpdater->addPhysicsBox( new osg::Box(osg::Vec3(), 0.99f),
                    osg::Vec3((float)i, 0.0f, (float)j+0.5f), osg::Vec3(), 1.0f );
            }
        }

        const unsigned inst_num = 24;
        for (unsigned i = 0; i < inst_num; ++i)
        {
            float const angle = 2.0f * /*cg::pif*/osg::PI * i / inst_num, radius = 400.f;
            osg::Vec3 pos(radius * sin (angle), radius * cos(angle), 0.f);

            const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(180.f * (i & 1)) - angle  , osg::Z_AXIS ); 


            osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(pos));
            //positioned->setDataVariance(osg::Object::STATIC);

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
            //rotated->setDataVariance(osg::Object::STATIC);

            positioned->addChild(rotated);
            //rotated->addChild(p_copy);
#ifdef DEPRECATED
            if(_rigidUpdater.valid())
             _rigidUpdater->addPhysicsAirplane( p_copy,
                pos, osg::Vec3(0,0,0), 800.0f );
#endif
            osg::Vec3 pos2( radius * sin (angle),   radius * cos(angle), 0.f);

#ifdef DEPRECATED
            if(_rigidUpdater.valid())
                _rigidUpdater->addPhysicsAirplane( p_copy,
                pos2, osg::Vec3(0,60,0), 1000.0f );
#endif
            // add it
            // _terrainRoot->addChild(positioned);  

            if (i==1) 
            {    
                auto manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( p_copy->getUpdateCallback() );
                if ( manager_ )
                {   

                    const osgAnimation::AnimationList& animations =
                        manager_->getAnimationList();

                    for ( unsigned int i=0; i<animations.size(); ++i )
                    {
                        const std::string& name = animations[i]-> getName();
                        if ( name==std::string("Default") )
                        {
                            auto anim = (osg::clone(animations[i].get(), "Animation_clone", osg::CopyOp::DEEP_COPY_ALL)); 
                            // manager->unregisterAnimation(animations[i].get());
                            // manager->registerAnimation  (anim/*.get()*/);

                            animations[i]->setPlayMode(osgAnimation::Animation::ONCE);                   
                            manager_->playAnimation( /*anim*/ animations[i].get(),2,2.0 );

                        }

                    }
                }


            }

            // FIXME при копировании начинаем падать кадров на 10
            p_copy = osg::clone(p_copy, osg::CopyOp::DEEP_COPY_ALL 
                & ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
                & ~osg::CopyOp::DEEP_COPY_ARRAYS
                & ~osg::CopyOp::DEEP_COPY_IMAGES
                & ~osg::CopyOp::DEEP_COPY_TEXTURES
                & ~osg::CopyOp::DEEP_COPY_STATESETS  
                & ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
                & ~osg::CopyOp::DEEP_COPY_UNIFORMS
                & ~osg::CopyOp::DEEP_COPY_DRAWABLES
                );

            // p_copy = osg::clone(p_copy,(const osg::CopyOp) (osg::CopyOp::DEEP_COPY_CALLBACKS & osg::CopyOp::DEEP_COPY_DRAWABLES) );
            
        }
    }

#if 0
    auto towbar = creators::createObject("towbar");
    osg::MatrixTransform* positioned = new osg::MatrixTransform;
    positioned->setMatrix(osg::Matrix::translate(osg::Vec3d(200,100,0)));
    positioned->addChild(towbar);
    addChild(towbar);
#endif

#if 0
    auto trap = creators::createObject("trap");
    _rigidUpdater->addVehicle(trap,
        osg::Vec3(250,750,00), osg::Vec3(0,30000,0), 1500.0f);

    auto buksir = creators::createObject("buksir");
    _rigidUpdater->addVehicle(buksir,
        osg::Vec3(270,750,00), osg::Vec3(0,30000,0), 6000.0f);


    auto cleaner = creators::createObject("cleaner");
    _rigidUpdater->addVehicle(cleaner,
        osg::Vec3(290,750,00), osg::Vec3(0,30000,0), 10000.0f);


    auto niva_cevrolet = creators::createObject("niva_chevrolet");
    _rigidUpdater->addVehicle(niva_cevrolet,
        osg::Vec3(310,750,00), osg::Vec3(0,30000,0), 1860.0f);


    auto pojarka = creators::createObject("pojarka");
    _rigidUpdater->addVehicle(pojarka,
        osg::Vec3(330,750,00), osg::Vec3(0,30000,0), 11200.0f);

  
    
    //pojarka->setNodeMask(0);
    //cleaner->setNodeMask(0);
    //niva_cevrolet->setNodeMask(0);
    //buksir->setNodeMask(0);
#endif

    const std::string v_name = "niva_chevrolet";

#if 0
    auto pojarka2 = creators::createObject(v_name);
    if(pojarka2)
	_rigidUpdater->addVehicle(pojarka2,
        osg::Vec3(330,750,00), osg::Vec3(0,30000,0), 11200.0f);

    auto pojarka_ctrl = creators::createObject(v_name);
    if(pojarka_ctrl)
	_rigidUpdater->addVehicle2(v_name,pojarka_ctrl,
        osg::Vec3(370,750,00), osg::Vec3(0,30000,0), 200.0f);  
#endif

    // _terrainRoot->addChild(_rigidUpdater->addGUIObject(poj));

    const bool add_vehicles = true;

    if(add_vehicles)
    {
        
    }

    if(_rigidUpdater.valid())
        _viewerPtr->addEventHandler( _rigidUpdater);



}

osg::Node*   Scene::load(std::string path,osg::Node* parent, uint32_t seed)
{
    osg::ref_ptr<osg::MatrixTransform> mt = nullptr ;

    if( path == "sfx//smoke.scg" )
    {
        mt = new osg::MatrixTransform;

        //bool got_phys_node=false;
        //while(0 != parent->getNumParents() && (got_phys_node = "phys_ctrl" != boost::to_lower_copy(parent->getName())))
        //{                  
        //    parent = parent->getParent(0);
        //}

        spark::spark_pair_t sp3 =  spark::create(spark::SMOKE,parent?parent->asTransform():nullptr);
        sp3.first->setName("fire");
        mt->addChild(sp3.first);
        
        addChild(mt);

        _viewerPtr->addEventHandler(sp3.second);
        return mt/*.release()*/;
    }

    osg::Node* obj = creators::createObject(path);

    if(obj)
    {
        mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",seed);

        mt->addChild( obj );

        osg::Node* root =  findFirstNode(obj,"root"); 
        root->setUserValue("id",seed);

        _terrainRoot->asGroup()->addChild(mt);
        
    }
    


    return mt;
}


void   Scene::onZoneChanged(int zone)
{
    const char* scene_name[] = {"empty","adler","sheremetyevo"};
    
    zone_to_reload_ = scene_name[zone];

    //_terrainRoot->removeChild(_terrainNode);
    //_terrainNode.release();
    //_terrainNode =  new avTerrain::Terrain (_terrainRoot);
    //_terrainNode->create(scene_name[zone]);

}

bool Scene::onEvent( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv )
{
    if (ea.getHandled() || ea.getEventType() != osgGA::GUIEventAdapter::KEYUP)
        return false;

    const int key = ea.getKey();

    if (key == osgGA::GUIEventAdapter::KEY_Escape)
    {
        if(_vis_settings_panel) _vis_settings_panel->set_visible(!_vis_settings_panel->visible());
        return true;
    }

    return false;
}