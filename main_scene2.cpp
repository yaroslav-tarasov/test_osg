#include "stdafx.h"
#include "Scene.h"
#include "creators.h"
#include "Terrain.h"

int main_scene2( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    avScene::Scene::Create(arguments);
    
    osg::ref_ptr<avScene::Scene> scene = avScene::Scene::GetInstance() ;
    
    return scene->GetViewer()->run();

}