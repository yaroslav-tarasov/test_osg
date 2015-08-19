#include "stdafx.h"
#include  "Visual.h"

#include "Scene.h"
#include "Logo.h"

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


Visual * Visual::CreateInstance()
{
    if (!m_pInstance)
        return new Visual();
    else
        return m_pInstance;
}
    
void Visual::Initialize(int argc, char** argv)
{
    avAssert(!m_bInitialized);

    // OSG graphics context
    osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = nullptr;//new osg::GraphicsContext::Traits();
#if 0
    //pTraits->inheritedWindowData           = new osgViewer::GraphicsWindowWin32::WindowData(hWnd);
    pTraits->alpha                         = 8;
    pTraits->setInheritedWindowPixelFormat = true;
    pTraits->doubleBuffer                  = true;
    pTraits->windowDecoration              = true;
    pTraits->sharedContext                 = NULL;
    pTraits->supportsResize                = true;
    pTraits->vsync                         = true;

    //RECT rect;
    //::GetWindowRect(hWnd, &rect);
    pTraits->x = 0;
    pTraits->y = 0;
    pTraits->width = 1920;//rect.right - rect.left + 1;
    pTraits->height = 1200;//rect.bottom - rect.top + 1;
#endif    

    osg::ArgumentParser arguments(&argc,argv);
    
    database::initDataPaths();

	osg::setNotifyLevel( osg::WARN );   /*INFO*//*NOTICE*//*WARN*/
	osg::setNotifyHandler( new LogFileHandler("goddamnlog.txt") );

	osg::notify(osg::INFO) << "Start Visual \n";

	osgDB::Registry::instance()->setOptions(new osgDB::Options("dds_flip dds_dxt1_rgba ")); 

    InitializeViewer( arguments, pTraits);
    
    //avScene::Logo::Create(_viewerPtr.get());
    avScene::Scene::Create(_viewerPtr.get());

    m_bInitialized = true;
}

void  Visual::Deinitialize()
{
    if (m_bInitialized)
    {
        m_bInitialized = false;
    }
}

void Visual::InitializeViewer(const osg::ArgumentParser& cArgs, osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr)
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

    osg::ArgumentParser  args(cArgs);
    // Create viewer and 
    _viewerPtr = new osgViewer::Viewer( args );

    _viewerPtr->apply(new osgViewer::SingleScreen(1));

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

    _viewerPtr->getCamera()->setClearColor(osg::Vec4(0.f, 0.f, 0.f, 1.f));
    //FIXME TODO //setProjectionMatrixFromConfig();

    _viewerPtr->getCamera()->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);    
    // _viewerPtr->getCamera()->setSmallFeatureCullingPixelSize(10.0F);

    //_viewerPtr->setSceneData( this );
    //_viewerPtr->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // TODO: enabled this for instructor tab, need implement special setting
    //_viewerPtr->setReleaseContextAtEndOfFrameHint(false); 

    // disable ESC key
    _viewerPtr->setKeyEventSetsDone(0);

}


void  Visual::Update()
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

void  Visual::Render()
{
    static bool run_once = false;
    
    if(!run_once)
    {
        osgViewer::Viewer& viewer =  *_viewerPtr.get(); 

        viewer.setReleaseContextAtEndOfFrameHint(false);

        viewer.realize();

        run_once = true;

    }

    
    if (m_bInitialized )
    {

        auto viewer =  _viewerPtr;

        if (!viewer->done())
        {
            frame_limiter fr(0);

            viewer->frame();
        }
    }
}

FIXME("I'm wrong")
double Visual::GetInternalTime()
{
    auto viewer = _viewerPtr;
    if (!viewer->done())
    {
        return viewer->getFrameStamp()->getSimulationTime();
    }

    return -1;
}


IVisual *  CreateVisual()
{
    return Visual::CreateInstance();
}
