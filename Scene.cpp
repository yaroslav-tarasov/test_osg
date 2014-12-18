#include "stdafx.h"
#include "Scene.h"
#include "creators.h"
#include "Ephemeris.h"
#include "sm/ShadowedScene.h"
#include "sm/ShadowMap.h"
#include "sm/ViewDependentShadowMap.h"
#include "Terrain.h"

using namespace avScene;

osg::ref_ptr<Scene>	 Scene::_scenePtr;

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

        // Release scene
        _scenePtr = NULL;
    }
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

    _viewerPtr->setSceneData( this );
    //_viewerPtr->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // TODO: enabled this for instructor tab, need implement special setting
    //_viewerPtr->setReleaseContextAtEndOfFrameHint(false); 

    // disable ESC key
    // _viewerPtr->setKeyEventSetsDone(0);

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
    manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
    manip->home(0);


    //
    // Add event handlers to the viewer
    //
    _viewerPtr->addEventHandler(new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()));
    _viewerPtr->addEventHandler(new osgViewer::StatsHandler);
    _viewerPtr->realize();
    
    //
    // Create terrain / shadowed scene
    //
    auto terrainRoot = createTerrainRoot();
    addChild(terrainRoot);
    
    _terrainNode =  new avTerrain::Terrain (terrainRoot);
    _terrainNode->create("adler");

    if(avTerrain::bi::getUpdater().valid())
        _viewerPtr->addEventHandler( avTerrain::bi::getUpdater().get() );

    //
    // Create ephemeris
    //                                                                       
    _ephemerisNode = new avSky::Ephemeris( this,
                                           _terrainNode.get(),
                                          [=](float illum){ _st->setNightMode(illum < .35);  } //  FIXME magic night value    
    );  

    if(_ls.valid()) _ephemerisNode->setSunLightSource(_ls);
    addChild( _ephemerisNode.get() );

    _viewerPtr->addEventHandler(_ephemerisNode->getEventHandler());
    
    //
    // Create weather
    //
    _weatherNode =  new osg::Group;
    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;		
    precipitationEffect->snow(0.3);
    precipitationEffect->setWind(osg::Vec3(0.2f,0.2f,0.2f));

    _weatherNode->addChild( precipitationEffect.get() );
    addChild( _weatherNode.get() );

    return true;
}


osg::Group *  Scene::createTerrainRoot()
{

#if defined(TEST_SHADOWS_FROM_OSG)

    const int fbo_tex_size = 1024*8;

    /*osg::ref_ptr<avShadow::ViewDependentShadowMap>*/ _st = new avShadow::ViewDependentShadowMap;

    osg::ref_ptr<avShadow::ShadowedScene> root
        = new avShadow::ShadowedScene(_st.get());  

    avShadow::ShadowSettings* settings = root->getShadowSettings();

    settings->setShadowMapProjectionHint(avShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);   //ORTHOGRAPHIC_SHADOW_MAP
    settings->setBaseShadowTextureUnit(5);
    settings->setMinimumShadowMapNearFarRatio(.5);
    //settings->setNumShadowMapsPerLight(/*numShadowMaps*/2);
    //settings->setMultipleShadowMapHint(testShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setMultipleShadowMapHint(avShadow::ShadowSettings::CASCADED);
    settings->setTextureSize(osg::Vec2s(fbo_tex_size,fbo_tex_size));
    //settings->setLightNum(2);
    settings->setMaximumShadowMapDistance(2000);
    settings->setShaderHint(avShadow::ShadowSettings::NO_SHADERS);

    _ls = new osg::LightSource;
    _ls->getLight()->setPosition(osg::Vec4(0, 0, 20, 0));
    //_ls->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    //_ls->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    _ls->getLight()->setLightNum(0);

    root->addChild(_ls.get());

#else
    osg::ref_ptr<osg::Group> root = new osg::Group;
#endif

    return root.release();
}