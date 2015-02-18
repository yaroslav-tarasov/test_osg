#include "stdafx.h"

#include "Scene.h"
#include "creators.h"
#include "Terrain.h"



int main_scene2( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

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
    
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\planes");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\vehicles");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\areas");

    avScene::Scene::Create(arguments/*,pTraits*/);
    
    osg::ref_ptr<avScene::Scene> scene = avScene::Scene::GetInstance() ;
    
    return scene->GetViewer()->run();

}

AUTO_REG(main_scene2)
