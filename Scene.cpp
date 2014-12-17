#include "stdafx.h"
#include "Scene.h"
#include "creators.h"
#include "Ephemeris.h"

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
    // Common nodes for scene, ships, anchors, buoys, etc.
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

    manip->setAcceleration(0);
    manip->setMaxVelocity(1);
    manip->setWheelMovement(10,true);
    // manip->setWheelMovement(0.001,true);
    _viewerPtr->setCameraManipulator(manip);
    manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
    manip->home(0);


    //
    // Add event handlers to the viewer
    //
    _viewerPtr->addEventHandler(new osgGA::StateSetManipulator(this->getOrCreateStateSet()/*getCamera()->getOrCreateStateSet()*/));
    _viewerPtr->addEventHandler(new osgViewer::StatsHandler);
    // _viewerPtr->realize();
    
    
    //
    // Create terrain / shadowed scene
    //
    bool overlay = false;
    // load the nodes from the command line arguments.
    auto model_parts  = creators::createModel(_ls, overlay, osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY);
    osg::Node* model = model_parts[0];

    addChild(model);

    //
    // Create ephemeris
    //
    _ephemerisNode = new avSky::Ephemeris( this, model->asGroup() );
    _ephemerisNode->setSunLightSource(_ls);
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
