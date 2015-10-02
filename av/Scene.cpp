#include "stdafx.h"
#include "av/precompiled.h"

#include "utils/high_res_timer.h"
#include "utils/pickhandler.h"

#if !defined(VISUAL_EXPORTS)
#include "phys/RigidUpdater.h"
#endif

#include "Logo.h"


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
      

#include "avShadows/ShadowedScene.h"
#include "avShadows/ShadowMap.h"
#include "avShadows/ViewDependentShadowMap.h"
#include "avSky/Sky.h"

#include "Scene.h"
#include "Lights.h"
#include "LightManager.h"
#include "Terrain.h"
#include "Environment.h"
#include "Ephemeris.h"
#include "Object.h"
#include "NavAid.h"


#include "application/panels/vis_settings_panel.h"
#include "application/main_window.h"
#include "application/menu.h"

#include "tests/common/CommonFunctions"
#include "tests/creators.h"

#include "utils/async_load.h"

//
//  ext
//
#include "spark/osgspark.h"

namespace gui 
{ 
    osgGA::GUIEventHandler*  createCEGUI(osg::Group* root, std::function<void()> init_gui_handler);
    void    releaseCEGUI();
}

using namespace avScene;


// Useless
#if 0
namespace {
    cg::transform_4 get_relative_transform( osg::Node* node, osg::Node* rel=nullptr )
    {
        osg::Matrix tr;
        osg::Node* n = node;
        //osg::Node* root = nullptr;

        while( 0 != n->getNumParents() && n->getName() != "phys_ctrl" && (rel?n != rel:true)  )
        {
            if(n->asTransform())
                if(n->asTransform()->asMatrixTransform())
                {
                    tr = n->asTransform()->asMatrixTransform()->getMatrix() * tr;
                }
                else
                {
                    osg::Matrix matrix;
                    const osg::PositionAttitudeTransform* pat = n->asTransform()->asPositionAttitudeTransform();
                    matrix.setTrans(pat->getPosition());
                    tr =  matrix * tr;
                }

            n = n->getParent(0);
        }

        if (rel == NULL || n == rel  )
            return from_osg_transform(tr);

        osg::Matrix tr_rel;
        n = rel;
        while(0 != n->getNumParents() && n->getName() != "phys_ctrl"/*root->getName()*/)
        {                  
            if(n->asTransform())
                if(n->asTransform()->asMatrixTransform())
                {
                    tr_rel = n->asTransform()->asMatrixTransform()->getMatrix() * tr_rel;
                }
                else
                {
                    osg::Matrix matrix;
                    const osg::PositionAttitudeTransform* pat = n->asTransform()->asPositionAttitudeTransform();
                    matrix.setTrans(pat->getPosition());
                    tr_rel = matrix * tr_rel;
                }

            n = n->getParent(0);
        }

        return from_osg_transform((osg::Matrix::inverse(tr_rel)) * tr);
    }

}
#endif

namespace {


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


}

class EnvHandler : public osgGA::GUIEventHandler
{

public:  
    EnvHandler(avSky::Sky* sky) 
        : _currCloud  (avSky::cirrus)
        , _intensivity(0.1)
        , _sky(sky)
    {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        auto _skyClouds  = avCore::GetEnvironment()->GetWeatherParameters().CloudType;
        const osg::Vec3f _color = osg::Vec3f(1.0,1.0,1.0);   // FIXME Whats color?

        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Insert)
            {
                //if(_skyClouds)
                {
                    int cc = _currCloud;cc++;
                    _currCloud = static_cast<avSky::cloud_type>(cc);
                    if(_currCloud >= avSky::clouds_types_num)
                        _currCloud = avSky::none;
                    
                    avCore::GetEnvironment()->m_WeatherParameters.CloudType = _currCloud;
                }
                return true;
            }
            else
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Up)
                {
                    
                    avCore::GetEnvironment()->m_WeatherParameters.CloudDensity = cg::bound(avCore::GetEnvironment()->GetWeatherParameters().CloudDensity +.1f,.0f,1.0f);
                    return true;
                }
                else
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Down)
                    {
                        avCore::GetEnvironment()->m_WeatherParameters.CloudDensity = cg::bound(avCore::GetEnvironment()->GetWeatherParameters().CloudDensity -.1f,.0f,1.0f);

                        return true;
                    }
                    else
                        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add)
                        {
                            avCore::Environment::TimeParameters &timeParameters = avCore::GetEnvironment()->m_TimeParameters;

                            if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                                timeParameters.Hour += 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                                timeParameters.Day += 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                               timeParameters.Month += 1;
                            else                                                                    // Increment by one minute
                                timeParameters.Minute += 1;

   
                            return true;
                        }

                        else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract)
                        {
                            avCore::Environment::TimeParameters &timeParameters = avCore::GetEnvironment()->m_TimeParameters;

                            if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                                timeParameters.Hour -= 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                                timeParameters.Day -= 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                                timeParameters.Month -= 1;
                            else                                                                    // Increment by one minute
                                timeParameters.Minute -= 1;

                            return true;
                        }
                        else
                        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_I)
                        {

                            //osgEphemeris::EphemerisData* data = ephem->getEphemerisData();

                            //data->turbidity += 1 ;

                            return true;
                        }
                        else
                        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_O)
                        {
                            //osgEphemeris::EphemerisData* data = ephem->getEphemerisData();
                            //// Increment by one minute
                            //data->turbidity -= 1 ;

                            return true;
                        }
                        else
                        if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Rightbracket )
                        { 
                            _intensivity += 0.1;
                            if(_intensivity > 1.0)
                                _intensivity = 1.0;
                            else
                               avCore::GetEnvironment()->m_WeatherParameters.FogDensity = _intensivity; 

                            return true;
                        } else
                        if (ea.getKey()== osgGA::GUIEventAdapter::KEY_Leftbracket)
                        { 
                            _intensivity -= 0.1;
                            if(_intensivity < 0.0) 
                                _intensivity = 0.0;
                            else
                                avCore::GetEnvironment()->m_WeatherParameters.FogDensity = _intensivity;

                            return true;
                        }

        }
        return false;
    } 

private:
    avSky::cloud_type                                  _currCloud;
    float                                             _intensivity;
    avSky::Sky *                                      _sky;

};




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
bool Scene::Create( osgViewer::Viewer* vw )
{
    _scenePtr = new Scene();

    if ( ! _scenePtr->Initialize( vw ))
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

#if !defined(VISUAL_EXPORTS)
        _scenePtr->_rigidUpdater->stopSession();
#endif
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
    _environmentNode = new Group();
    addChild(_environmentNode.get());

    // Common nodes for scene etc.
    _commonNode = new Group();

    osg::StateSet * pCommonStateSet = _commonNode->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");

    addChild(_commonNode.get());

    // Add backface culling to the whole bunch
    osg::StateSet * pSS = getOrCreateStateSet();
    pSS->setNestRenderBins(false);
    pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    // disable alpha writes for whole bunch
    pSS->setAttribute(new osg::ColorMask(true, true, true, false)); 


}

//////////////////////////////////////////////////////////////////////////
Scene::~Scene()
{
}

namespace
{

osg::Node * createScene()
{
    osg::Group* root = new osg::Group;
    // global sky state-set
    osg::StateSet * pSkyStateSet = root->getOrCreateStateSet();

    pSkyStateSet->setNestRenderBins(false);

    // disable back face culling
    pSkyStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // disable writing to depth buffer
    osg::Depth * pDepth = new osg::Depth;
    //pDepth->setWriteMask(false);
    pSkyStateSet->setAttribute(pDepth);

    osg::Sphere* sphere    = new osg::Sphere( osg::Vec3( 0.f, 0.f, 0.f ), 0.25f );
    osg::ShapeDrawable* sd = new osg::ShapeDrawable( sphere );
    sd->setColor( osg::Vec4( 1.f, 0.f, 0.f, 1.f ) );
    sd->setName( "A nice sphere" );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( sd );

    // Set material for basic lighting and enable depth tests. Else, the sphere
    // will suffer from rendering errors.
    {
        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        osg::Material* material = new osg::Material;

        material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );	

        stateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
    }
    
    root->addChild(geode);
    root->setCullingActive(false);

    return root;
}

}

#define MANIPS

//////////////////////////////////////////////////////////////////////////
bool Scene::Initialize( osgViewer::Viewer* vw)
{
    osg::StateSet* pGlobalStateSet = getOrCreateStateSet();
    osg::StateSet* pCommonStateSet = getCommonNode()->getOrCreateStateSet();
    
    _viewerPtr = vw;            

    _viewerPtr->setSceneData( this );
   
    //
    // Set up the camera manipulators.
    //

#ifdef MANIPS
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


				this->_mw = app::create_main_win();
				app::menu_ptr fm = this->_mw->add_main_menu("File");
				fm->add_string("Exit" , boost::bind(&Scene::onExit,this)); // [&]() { /*exit(0);*/});

				app::menu_ptr vm = this->_mw->add_main_menu("View");
				vm->add_string("Lights" , [=]() {  });

				this->_mw->set_visible(false);

				this->_vis_settings_panel = app::create_vis_settings_panel( zones_ );
                this->_vis_settings_panel->subscribe_zone_changed(boost::bind(&Scene::onZoneChanged,this,_1));
                this->_vis_settings_panel->subscribe_exit_app    (boost::bind(&Scene::onExit,this));
				this->_vis_settings_panel->subscribe_set_lights(boost::bind(&Scene::onSetLights,this,_1));
				this->_vis_settings_panel->subscribe_set_map(boost::bind(&Scene::onSetMap,this,_1));
                this->_vis_settings_panel->set_light(true);
            }
        } )
    );


    _viewerPtr->addEventHandler( new osgViewer::WindowSizeHandler );
    _viewerPtr->addEventHandler( new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()) );
    _viewerPtr->addEventHandler( new osgViewer::StatsHandler );
    _viewerPtr->addEventHandler( _pickHandler );    
    _viewerPtr->addEventHandler( avGUI::makeGUIEventHandlerImpl(this, &Scene::onEvent));
    //_viewerPtr->realize();
	
    addChild(_pickHandler->getOrCreateSelectionBox()); 
#endif


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

    _terrainRoot = createTerrainRoot();
    addChild(_terrainRoot);
  
    //std::string scene_name("empty"); // "empty","adler" ,"sheremetyevo"
    //_terrainNode =  new avTerrain::Terrain (_terrainRoot);
    //_terrainNode->create(scene_name);

       
    osg::Node* ct =  nullptr;// findFirstNode(_terrainNode,"camera_tower");
    
    

#if 1
#ifdef MANIPS
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
#endif
#endif

#if 1 


#ifdef ORIG_EPHEMERIS
    //
    // Create ephemeris
    //                                                                       
    _ephemerisNode = new avSky::Ephemeris( this
                                          , _terrainNode.get()
                                          ,[=](float illum){ if(_st!=0) _st->setNightMode(illum < 0.8);} //  FIXME magic night value    
                                          ,[this](float fog_vr){
                                            BOOST_FOREACH( auto g, this->_lamps)
                                            {
                                                 dynamic_cast<osgSim::LightPointNode*>(g.get())->setMaxVisibleDistance2(fog_vr * fog_vr);
                                            }
                                          }
    
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
//////////////////////////////////////////////////////    
    FIXME(Many light sources)
#if 0
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;

    osg::Vec4f lightPosition2 (osg::Vec4f(200.0,100.0,50.0,0.0f));
    osg::ref_ptr<osg::Light> myLight2 = new osg::Light;

    myLight2->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.9f,0.9f,0.85f,1.0f));
    myLight2->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    myLight2->setConstantAttenuation(1.0f);

    myLight2->setLightNum(2);
    myLight2->setPosition(lightPosition2);
    ls->setLight(myLight2.get());
    if(_terrainRoot) _terrainRoot->addChild(ls.get());
#endif
/////////////////////////////////////////////////////////////////

    _viewerPtr->addEventHandler(_ephemerisNode->getEventHandler());
   


#else  

    avCore::Environment::Create();
  
    // Create sky
    //
    _Sky = new avSky::Sky( this );
    /*_environmentNode->*/addChild( _Sky.get() );
  
    if( _Sky->getSunLightSource())
    {
        _ls =_Sky->getSunLightSource();  
        if(_terrainRoot) _terrainRoot->addChild(_ls.get());
    }

    _viewerPtr->addEventHandler(new EnvHandler(_Sky));

#endif


    LightManager::Create();

    //
    // Dynamic lights manager
    //
    

    addChild(LightManager::GetInstance());

    //
    // Create dynamic lights
    //
    _lights = new Lights();
    _environmentNode->addChild(_lights.get());

   
    
    //
    // Create weather
    //
    _weatherNode =  new osg::Group;
    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;		
    precipitationEffect->snow(0.0);
    precipitationEffect->setWind(osg::Vec3(0.2f,0.2f,0.2f));

    _weatherNode->addChild( precipitationEffect.get() );
    addChild( _weatherNode.get() );


    //
    // Init physic updater
    //

    createObjects();

#if 1 

    light_map = createLightMapRenderer(this);
    addChild( light_map );

#endif

    FIXME(140 shaders version needed);

#if GLSL_VERSION > 150
    osgViewer::Viewer::Windows windows;
    _viewerPtr->getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        osg::State *s=(*itr)->getState();
        s->setUseModelViewAndProjectionUniforms(true);
        s->setUseVertexAttributeAliasing(true);
    }
#endif 

#else
    _ls = new osg::LightSource;
    // _ls->getLight()->setLightNum(0);
    if(_terrainRoot)
        _terrainRoot->addChild(_ls.get());
    
    osg::Vec4f lightPosition2 (osg::Vec4f(-200.0,-100.0,-300.0,0.0f));
    osg::ref_ptr<osg::Light> myLight2 = new osg::Light;
    
    myLight2->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.9f,0.9f,0.85f,1.0f));
    myLight2->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    myLight2->setConstantAttenuation(1.0f);

    myLight2->setLightNum(0);
    myLight2->setPosition(lightPosition2);
    _ls->setLight(myLight2.get());
    _ls->setLocalStateSetModes(osg::StateAttribute::ON); 

    /*_terrainRoot->*/addChild(::createScene());
#endif



    return true;
}


osg::Group*  Scene::createTerrainRoot()
{
    osg::Group* tr;
//#undef TEST_SHADOWS_FROM_OSG
#if defined(TEST_SHADOWS_FROM_OSG)

    const int fbo_tex_size = 1024*4;

    _st = new avShadow::ViewDependentShadowMap;

     tr = new avShadow::ShadowedScene(_st.get());  

    avShadow::ShadowSettings* settings = dynamic_cast<avShadow::ShadowedScene*>(tr)->getShadowSettings();

    settings->setShadowMapProjectionHint(avShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);   //ORTHOGRAPHIC_SHADOW_MAP
    settings->setBaseShadowTextureUnit(BASE_SHADOW_TEXTURE_UNIT);
    settings->setMinimumShadowMapNearFarRatio(0.5);
    //settings->setNumShadowMapsPerLight(/*numShadowMaps*/2);
    //settings->setMultipleShadowMapHint(avShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setMultipleShadowMapHint(avShadow::ShadowSettings::CASCADED);
    settings->setTextureSize(osg::Vec2s(fbo_tex_size,fbo_tex_size));
    //settings->setLightNum(2);
    settings->setMaximumShadowMapDistance(1500/*150*/);
    settings->setShaderHint(avShadow::ShadowSettings::NO_SHADERS);
	//settings->setCastsShadowTraversalMask(cCastsShadowTraversalMask);
	//settings->setReceivesShadowTraversalMask(cReceivesShadowTraversalMask); 

#else
    tr = new osg::Group;
#endif     
    return tr;

}

void Scene::createObjects()
{

#if !defined(VISUAL_EXPORTS)
    _rigidUpdater = new bi::RigidUpdater( _terrainRoot->asGroup() 
        ,[&](osg::MatrixTransform* mt){ 
            if(!findFirstNode(mt,"fire"))
            {
                spark::spark_pair_t sp =  spark::create(spark::FIRE,mt);
                sp.first->setName("fire");
                mt->addChild(sp.first);
                this->_viewerPtr->addEventHandler(sp.second);
            }
    }
    );
#endif

    //auto heli = creators::applyBM(creators::loadHelicopter(),"mi_8",true);
    //_terrainRoot->addChild(heli);


    //if(_rigidUpdater.valid())
    //    _rigidUpdater->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );

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
#endif

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
#if !defined(VISUAL_EXPORTS)
    if(_rigidUpdater.valid())
    {
        _viewerPtr->addEventHandler( _rigidUpdater);

        conn_holder_ << _pickHandler->subscribe_choosed_point(boost::bind(&bi::RigidUpdater::handlePointEvent, _rigidUpdater.get(), _1));
        conn_holder_ << _pickHandler->subscribe_selected_node(boost::bind(&bi::RigidUpdater::handleSelectObjectEvent, _rigidUpdater.get(), _1));

        conn_holder_ << _rigidUpdater->subscribe_selected_object_type(boost::bind(&PickHandler::handleSelectObjectEvent, _pickHandler.get(), _1));
        _rigidUpdater->setTrajectoryDrawer(new TrajectoryDrawer(this,TrajectoryDrawer::LINES));
    }
#endif

}

osg::Node*   Scene::load(std::string path,osg::Node* parent, uint32_t seed)
{
    using namespace creators;

    // osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform; // nullptr ;
    
    mt_.push_back(new osg::MatrixTransform); 
    
    LogInfo("Scene::load enter " << path);

    if( path == "sfx//smoke.scg" )
    {
        // mt = new osg::MatrixTransform;

        //bool got_phys_node=false;
        //while(0 != parent->getNumParents() && (got_phys_node = "phys_ctrl" != boost::to_lower_copy(parent->getName())))
        //{                  
        //    parent = parent->getParent(0);
        //}

        spark::spark_pair_t sp3 =  spark::create(spark::SMOKE,parent?parent->asTransform():nullptr);
        sp3.first->setName("fire");
        mt_.back()->addChild(sp3.first);
        
        addChild(mt_.back());
		
		//mt_.back()->setNodeMask(  cCastsShadowTraversalMask );

        _viewerPtr->addEventHandler(sp3.second);
        return mt_.back()/*.release()*/;


    }

    if (path == "adler" || path == "sheremetyevo" || path == "minsk" )
    {
        //assert(_terrainRoot->removeChild(_terrainNode));
        //_terrainNode.release();
        _terrainNode =  new avTerrain::Terrain (this);
        _terrainNode->create(path);
        
        _terrainRoot->asGroup()->addChild(_terrainNode);

         /*_commonNode*//*this*/_terrainRoot->setCullCallback(new DynamicLightsObjectCull(GlobalInfluence));

        return _terrainNode;
    }

    auto  wf =  [this,&seed](std::string path, osg::MatrixTransform* mt) {
    
   /* osg::ref_ptr<osg::MatrixTransform> mt = mt_;*/

    using namespace creators;
    
    osg::Node* obj = creators::createObject(path);
    

    if(obj)
    {
        // mt = new osg::MatrixTransform;
        mt->setName("phys_ctrl");
        mt->setUserValue("id",seed);

        mt->addChild( obj );

        osg::Node* root =  findFirstNode(obj,"root"); 
        root->setUserValue("id",seed);


        
        if(mt!=nullptr)
        {
            
            osg::Node* sl =  findFirstNode(mt,"steering_lamp",findNodeVisitor::not_exact);
            osg::Node* pat =  findFirstNode(mt,"pat",findNodeVisitor::not_exact);

            const auto offset =  pat->asTransform()->asPositionAttitudeTransform()->getPosition();

            if(sl)
            {
                avScene::LightManager::Light data;
                data.transform  = mt;  

                data.spotFalloff = cg::range_2f(osg::DegreesToRadians(25.f), osg::DegreesToRadians(33.f));
                data.distanceFalloff = cg::range_2f(75.f, 140.f);

				data.color.r = 0.92f;
                data.color.g = 0.92f;
                data.color.b = 0.85f;

                FIXME(  Damned offset  );
                //cg::transform_4 tr = get_relative_transform(sl);
                data.position =  from_osg_vector3(sl->asTransform()->asMatrixTransform()->getMatrix().getTrans() 
                                                  + offset);

                osg::Quat      rot   = sl->asTransform()->asMatrixTransform()->getMatrix().getRotate();
                cg::quaternion orien = from_osg_quat(rot);
                cg::cpr        cr    = orien.cpr(); 

                const float heading = osg::DegreesToRadians(cr.course);
                const float pitch = osg::DegreesToRadians(/*cr.pitch*/15.f);

                data.direction = cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

                data.active = true;

                avScene::LightManager::GetInstance()->addLight(avScene::LightManager::GetInstance()->genUID(), data);

            }


            findNodeVisitor::nodeNamesList list_name;

            //osgSim::LightPointNode* obj_light =  new osgSim::LightPointNode;
            NavAidGroup*  obj_light  =  new NavAidGroup; 

            const char* names[] =
            {
                "port",
                "starboard",
                "tail",
                "steering_lamp",
                "strobe_",
                "landing_lamp",
                "back_tail",
                // "navaid_",
            };

            for(int i=0; i<sizeof(names)/sizeof(names[0]);++i)
            {
                list_name.push_back(names[i]);
            }

            findNodeVisitor findNodes(list_name,findNodeVisitor::not_exact); 
            root->accept(findNodes);

            findNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();

            auto shift_phase = cg::rand(cg::range_2(0, 255));

            osgSim::Sector* sector = new osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));

            for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
            {
                osgSim::LightPoint pnt;
                bool need_to_add = false;
                avScene::LightManager::Light data;

                if((*it)->getName() == "tail")
                { 
                    pnt._color      = creators::white_color;
                    need_to_add     = true;
                }

                if((*it)->getName() == "port")
                {   
                    pnt._color      = green_color;
                    need_to_add     = true;
                    pnt._sector = sector;

                    data.transform  = mt;  

                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(1.5f, 13.f);
                    
                    data.color  *= 0.01;

                    FIXME( Damned offset )
                    data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() 
                        + offset);

                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f/*-90.f*/);

                    data.direction = cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

                    data.active = true;

                    //avScene::LightManager::GetInstance()->addLight(data);

                }

                if((*it)->getName() == "starboard") 
                {
                    pnt._color = red_color;
                    need_to_add     = true;
                    pnt._sector = sector;

                    data.transform  = mt;  

                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(1.5f, 13.f);
                    
                    FIXME( Damned offset )
                    data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() 
                        + offset);

                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f/*-90.f*/);

                    data.direction = cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

                    data.active = true;
                }


                if(boost::starts_with((*it)->getName(), "strobe_")) 
                {
                    pnt._color  = white_color;
                    pnt._blinkSequence = new osgSim::BlinkSequence;
                    pnt._blinkSequence->addPulse( 0.05,
                        osg::Vec4( 1., 1., 1., 1. ) );

                    pnt._blinkSequence->addPulse( 1.5,
                        osg::Vec4( 0., 0., 0., 0. ) );

                    pnt._sector = new osgSim::AzimSector(-osg::inDegrees(170.0),-osg::inDegrees(10.0),osg::inDegrees(90.0));

                    pnt._blinkSequence->setPhaseShift(shift_phase);
                    need_to_add     = true;

                    data.transform  = mt;  
                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(3.5f, 15.f);//cg::range_2f(3.5f, 40.f);

                    FIXME( Damned offset )
                        data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() 
                        + offset);

                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f/*-90.f*/);

                    data.direction = cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

                    data.active = true;
                }

                pnt._position = (*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans();
                pnt._radius = 0.2f;
                if(need_to_add)
                {
                    data.color.r = pnt._color.r();
                    data.color.g = pnt._color.g();
                    data.color.b = pnt._color.b();

                    FIXME( Need to be normalized )
                        data.color  *= 0.01;

                    obj_light->addLight(pnt, data);
                }
            }

            if(wln_list.size()>0)
                root->asGroup()->addChild(obj_light);


        }

        _terrainRoot->asGroup()->addChild(mt);
		
		//mt->setNodeMask(  cCastsShadowTraversalMask | cReceivesShadowTraversalMask );

        object_loaded_signal_(seed);

    } };

#ifdef ASYNC_OBJECT_LOADING
    _lnt =   new utils::LoadNodeThread ( boost::bind<void>(wf,path,mt_.back().get()) );
#else
    wf(path,mt_.back().get());
#endif

    LogInfo("Scene::load exit " << path);

    return mt_.back();
}

void   Scene::onSetMap(float val)
{
	_terrainNode->setGrassMapFactor(val);
}

void   Scene::onSetLights(bool on)
{
	if (_lights.valid())
    {
        _lights->setNodeMask(on?0xffffffff:0);
        LightManager::GetInstance()->setNodeMask(on?0xffffffff:0);
    }
}

void   Scene::onZoneChanged(int zone)
{
    const char* scene_name[] = {"empty","adler","sheremetyevo","minsk"};
    
    zone_to_reload_ = scene_name[zone];

    //_terrainRoot->removeChild(_terrainNode);
    //_terrainNode.release();
    //_terrainNode =  new avTerrain::Terrain (_terrainRoot);
    //_terrainNode->create(scene_name[zone]);

}

bool Scene::onEvent( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & /*aa*/, osg::Object * /*obj*/, osg::NodeVisitor * /*nv*/ )
{
    if (ea.getHandled() || ea.getEventType() != osgGA::GUIEventAdapter::KEYUP)
        return false;

    const int key = ea.getKey();

    if (key == osgGA::GUIEventAdapter::KEY_Escape)
    {
		if(_mw) _mw->set_visible(!_mw->visible());

        return true;
    }

    return false;
}

void Scene::onExit()
{
    _viewerPtr->setDone(true);
}