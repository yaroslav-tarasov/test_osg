#include  "Visual.h"

#include "avScene/Scene.h"
#include "avCore/Logo.h"
#include "avCore/Object.h"

#include <osgViewer/api/Win32/GraphicsWindowWin32>

using namespace avCore;

namespace
{
	class LogFileHandler : public osg::NotifyHandler
	{
	public:
		LogFileHandler( const std::string& file )
		{ _log.open( file.c_str() ); }
		virtual ~LogFileHandler() { _log.close(); }
		virtual void notify(osg::NotifySeverity severity,
			const char* msg)
		{ 
			static std::string str_severity[] =
			{
				"ALWAYS",
				"FATAL",
				"WARN",
				"NOTICE",
				"INFO",
				"DEBUG_INFO",
				"DEBUG_FP"
			};

			_log << str_severity[severity] << ": " << msg;

		}
	protected:
		std::ofstream _log;
	};
}

namespace av
{



Visual * Visual::m_pInstance = nullptr;



Visual::Visual()
    : m_bInitialized(false)
{
    avAssert(m_pInstance == nullptr);
    m_pInstance = this;
}

Visual::~Visual()
{
    avAssert(m_pInstance != nullptr);
    m_pInstance = nullptr;
}

osgViewer::Viewer* Visual::GetViewer() const
{
    return  _viewerPtr.get();
}

Visual * Visual::CreateInstance()
{
    if (!m_pInstance)
        return new Visual();
    else
        return m_pInstance;
}

namespace {
	osg::Node * createScene()
	{
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

		return geode;
	}
}


void Visual::Initialize()
{
    avAssert(!m_bInitialized);
    
#if 0
    osgDB::Registry::instance()->setBuildKdTreesHint(osgDB::ReaderWriter::Options::BUILD_KDTREES);
#endif

    // OSG graphics context
    osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = nullptr; //new osg::GraphicsContext::Traits();

    float width    = osg::DisplaySettings::instance()->getScreenWidth();
    float height   = osg::DisplaySettings::instance()->getScreenHeight();
    float distance = osg::DisplaySettings::instance()->getScreenDistance();

#if 0
    const std::string version( "4.3" );
    pTraits = new osg::GraphicsContext::Traits();
    // pTraits->inheritedWindowData           = new osgViewer::GraphicsWindowWin32::WindowData(hWnd);
    pTraits->alpha                         = 8;
    pTraits->setInheritedWindowPixelFormat = true;
    pTraits->doubleBuffer                  = true;
    pTraits->windowDecoration              = true;
    pTraits->sharedContext                 = NULL;
    pTraits->supportsResize                = true;
    pTraits->vsync                         = true;
    pTraits->useMultiThreadedOpenGLEngine  = true;
    pTraits->glContextVersion = version;

    //RECT rect;
    //::GetWindowRect(hWnd, &rect);
    pTraits->x = 1920;
    pTraits->y = 0;
    pTraits->width = 1920;//rect.right - rect.left + 1;
    pTraits->height = 1200;//rect.bottom - rect.top + 1;
#endif    

    ::Database::initDataPaths();
	avCore::Database::Create();

	osg::setNotifyLevel( osg::INFO );   /*INFO*//*NOTICE*//*WARN*//*DEBUG_INFO*/
	osg::setNotifyHandler( new LogFileHandler("goddamnlog.txt") );

	osg::notify(osg::INFO) << "Start Visual \n";

	osgDB::Registry::instance()->setOptions(new osgDB::Options("dds_flip dds_dxt1_rgba ")); 
    
    
    InitializeViewer( pTraits);
    
    avScene::Logo::Create(_viewerPtr.get());
	
    CreateScene();

    m_bInitialized = true;
}

void  Visual::Deinitialize()
{
    if (m_bInitialized)
    {
        m_bInitialized = false;
    }
}

void Visual::InitializeViewer(osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr)
{
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
    _viewerPtr = new osgViewer::Viewer();

#ifdef STATIC_VISUAL_API
    _viewerPtr->apply(new osgViewer::SingleScreen(0));
#endif

    // Set up camera
    if ( cTraitsPtr.valid() == true )
    {
        osg::ref_ptr<osg::GraphicsContext> cGraphicsContextPtr = osg::GraphicsContext::createGraphicsContext( cTraitsPtr.get() );

        _viewerPtr->getCamera()->setGraphicsContext(cGraphicsContextPtr.get());
        _viewerPtr->getCamera()->setViewport(new osg::Viewport(0, 0, cTraitsPtr->width, cTraitsPtr->height ));
    } 
    else if (getenv("OSG_SCREEN") == NULL)
    {
       //_viewerPtr->setUpViewInWindow(20, 20, 820, 620);
    }
    
    //float left, right, bottom, top;

    //osg::ref_ptr<osgViewer::DepthPartitionSettings> dps = new osgViewer::DepthPartitionSettings(osgViewer::DepthPartitionSettings::FIXED_RANGE);
    //dps->_zFar = 30000.0;
    //dps->_zNear = 4.0;
    //dps->_zMid = (dps->_zFar - dps->_zNear) * .5;


    //_viewerPtr->setUpDepthPartition(dps);
    
    _viewerPtr->getCamera()->setClearColor(osg::Vec4(0.f, 0.f, 0.f, 1.f));
    //FIXME TODO //setProjectionMatrixFromConfig();

#if 0     
    float fVertAspect = float(1980) / float(1200);
    float m_fLeft   = -1 ;
    float m_fRight  = +1 ;
    float m_fBottom = -1 ;
    float m_fTop    = +1 ;
    float m_fHalfTanH = 0.5f * (m_fRight - m_fLeft);
    m_fLeft   = -m_fHalfTanH;
    m_fRight  = +m_fHalfTanH;
    m_fBottom = -m_fHalfTanH * fVertAspect;
    m_fTop    = +m_fHalfTanH * fVertAspect;

    _viewerPtr->getCamera()->setProjectionMatrixAsFrustum(m_fLeft,m_fRight,m_fBottom,m_fTop,4.0,7000.0);
#endif
    
#if 0
    
    float fZoom = 1.0f; 

    float zNear  =  1.0f; 
    float zFar   =  40000.0f;
    
    float zoom   = (fZoom > 1.0f) ? fZoom : 1.0f;
    
    float                           fFOVHor[2];
    float                           fFOVVer[2];

    const float fVertAspect = 1200.f/1920.f;
    const float half_fovhor = 20.f;
    const float half_fovver = half_fovhor * fVertAspect;

    fFOVHor[0] = osg::inDegrees(-half_fovhor);
    fFOVHor[1] = osg::inDegrees( half_fovhor);
    fFOVVer[0] = osg::inDegrees(-half_fovver);
    fFOVVer[1] = osg::inDegrees( half_fovver);
 	
    float left, right, bottom, top;

    left   = zNear * tan(0.5*(fFOVHor[0] - fFOVHor[1])/zoom /*+ cVisualConfig.fTwist[0]*/);
    right  = zNear * tan(0.5*(fFOVHor[1] - fFOVHor[0])/zoom /*+ cVisualConfig.fTwist[0]*/);
    bottom = zNear * tan(fFOVVer[0]/zoom);
    top    = zNear * tan(fFOVVer[1]/zoom);
    //_viewerPtr->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    _viewerPtr->getCamera()->setProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);
#endif

    _viewerPtr->getCamera()->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);    
   // _viewerPtr->getCamera()->setSmallFeatureCullingPixelSize(5.0F);

    //_viewerPtr->setSceneData( this );
    _viewerPtr->setThreadingModel(osgViewer::Viewer::SingleThreaded /*ThreadPerCamera*/);

    // TODO: enabled this for instructor tab, need implement special setting
    //_viewerPtr->setReleaseContextAtEndOfFrameHint(false); 

    // disable ESC key
    _viewerPtr->setKeyEventSetsDone(0);

    //_viewerPtr->setRunFrameScheme( osgViewer::ViewerBase::ON_DEMAND );
}

void Visual::CreateScene()
{
      avScene::Scene::Create(_viewerPtr.get());
}


void   Visual::SetPosition (const cg::point_3f& pos, const cg::quaternion&  orien)
{
    point_3 forward_dir =  cg::normalized_safe(orien.rotate_vector(point_3(0, 1, 0))) ;
	point_3 look_dir = forward_dir + pos;// orien.rotate_vector(forward_dir + pos);
	avScene::GetScene()->setHomePosition(to_osg_vector3(pos),to_osg_vector3(look_dir));
}


void Visual::EndSceneCreation()
{
    _viewerPtr->setSceneData( avScene::Scene::GetInstance() );
    avScene::Logo::Release(_viewerPtr.get());
}

bool   Visual::Done()
{
     return _viewerPtr->done();
}



void   Visual::SetDone(bool d)
{
    return _viewerPtr->setDone(d);
}



void  Visual::Update(double ref_time)
{
    if (m_bInitialized )
    {
    }
}

struct frame_limiter
{
    frame_limiter(double max_frame_rate)
        : _runMaxFrameRate(max_frame_rate)
        , startFrameTick(osg::Timer::instance()->tick())
        , _minFrameTime(_runMaxFrameRate>0.0 ? 1.0/_runMaxFrameRate : 0.0)
    {}

    ~frame_limiter()
    {
        // work out if we need to force a sleep to hold back the frame rate
        osg::Timer_t endFrameTick = osg::Timer::instance()->tick();
        double frameTime = osg::Timer::instance()->delta_s(startFrameTick, endFrameTick);
        if (frameTime < _minFrameTime) OpenThreads::Thread::microSleep(static_cast<unsigned int>(1000000.0*(_minFrameTime-frameTime)));
    }

    osg::Timer_t startFrameTick;
    double       _runMaxFrameRate;
    double       _minFrameTime;
};

void Visual::PreUpdate   (double ref_time)
{
     if(avScene::GetScene())
         avScene::GetScene()->PreUpdate();

     ObjectManager::get().PreUpdate();
}

 
namespace 
{
 class FreeViewControl : public osgGA::GUIEventHandler
 {

 public:  
	 FreeViewControl(IFreeViewControl *  fvc) 
		 : _fvc (fvc)
	 {}

	 virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	 {
		 if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		 {
			 if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Space)
			 {
				  _fvc->FreeViewOff();
				 return false;
			 }
			 else
			 if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Return)
			 {
				 _fvc->FreeViewOn();
				 return false;
			 }
			 
		 }

		 return false;
	 } 

 private:
	IFreeViewControl *                          _fvc;

 };

}


void  Visual::SetFreeViewControl(IFreeViewControl * fvc) 
{
	 _viewerPtr->addEventHandler(new FreeViewControl(fvc));
}

void  Visual::Render(double ref_time)
{
    static bool run_once = false;
    
    if (m_bInitialized )
    {
        if(!run_once)
        {
            osgViewer::Viewer& viewer =  *_viewerPtr.get(); 

            _viewerPtr->setReleaseContextAtEndOfFrameHint(false);

            viewer.realize();

            run_once = true;

        }

        PreUpdate   (ref_time);

        if (!_viewerPtr->done())
        {
            //frame_limiter fr(0);

			if(ref_time<0.0)
				_viewerPtr->frame();
			else
				_viewerPtr->frame(ref_time);
        }
    }
}

FIXME("I'm wrong")
double Visual::GetInternalTime()
{
    auto viewer = _viewerPtr;
    if (viewer && !viewer->done())
    {
        return viewer->getFrameStamp()->getSimulationTime();
    }

    return -1;
}

IScenePtr Visual::GetScene(void) const
{
    return avScene::GetScene();
}

IVisual *  CreateVisual()
{
    IVisual * iv = Visual::CreateInstance();
    iv->Initialize();
    return iv;
}
         

}