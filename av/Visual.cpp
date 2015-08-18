#include "stdafx.h"
#include  "Visual.h"

#include "Scene.h"

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

#if 0
    // OSG graphics context
    osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits();
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

	osg::setNotifyLevel( osg::WARN/*INFO*//*NOTICE*//*WARN*/ );
	osg::setNotifyHandler( new LogFileHandler("goddamnlog.txt") );

	osg::notify(osg::INFO) << "Start Visual \n";

	osgDB::Registry::instance()->setOptions(new osgDB::Options("dds_flip dds_dxt1_rgba ")); // dds_flip dds_dxt1_rgba  

    avScene::Scene::Create(arguments/*,pTraits*/);

    m_bInitialized = true;
}

void  Visual::Deinitialize()
{
    if (m_bInitialized)
    {
        m_bInitialized = false;
    }
}

void  Visual::Update()
{
    if (m_bInitialized )
    {
    }
}

void  Visual::Render()
{
    if (m_bInitialized )
    {

        auto viewer = avScene::Scene::GetInstance()->GetViewer();

        if (!viewer->done())
        {
            viewer->frame();
        }
    }
}

FIXME("I'm wrong")
double Visual::GetInternalTime()
{
    auto viewer = avScene::Scene::GetInstance()->GetViewer();
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
