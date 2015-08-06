#include "stdafx.h"

#include "av/Scene.h"
#include "creators.h"
#include "av/Terrain.h"

int av_scene( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    logger::need_to_log(/*true*/);

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

    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    

    //do
    //{
        avScene::Scene::Create(arguments/*,pTraits*/);
    
        auto viewer = avScene::Scene::GetInstance()->GetViewer();

        while (!viewer->done())
        {
             viewer->frame();
        }

        //avScene::Scene::Release();

    //} while (!avScene::Scene::zoneToReload().empty());
    


    return 0; //scene->GetViewer()->run();

}

AUTO_REG(av_scene)
