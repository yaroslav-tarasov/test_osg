#include "stdafx.h"

#define SB_NO_PYTHON

#pragma comment(lib, "SmartBody.lib")

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>

#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBPython.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBSceneListener.h>

int main_sb( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
    scene->setMediaPath( "./smartbody/" );   // data/
    scene->addAssetPath( "motion", "ChrBrad" );
    scene->addAssetPath( "mesh", "mesh");
    scene->addAssetPath( "script", "scripts");
    scene->loadAssets();
    
    int numMotions = scene->getNumMotions();
    std::cout << "Loaded motions: " << numMotions << std::endl;
    
    SmartBody::SBCharacter* character = scene->createCharacter( "mycharacter", "" );
    SmartBody::SBSkeleton* skeleton = scene->createSkeleton( "ChrBrad.sk" );
    character->setSkeleton( skeleton );
    character->createStandardControllers();
    
    SmartBody::SBSimulationManager* sim = scene->getSimulationManager();
    sim->setupTimer();
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    
    osgViewer::Viewer viewer;
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    viewer.setUpViewOnSingleScreen( 1 );
    
    std::string ret =  scene->getBmlProcessor()->execBML( "mycharacter", "<body posture=\"ChrBrad@Idle01\"/>" );
    sim->start();
    while ( !viewer.done() )
    {
        scene->update();
        sim->updateTimer();
        viewer.frame();
    }
    sim->stop();
    return 0;
}


 AUTO_REG(main_sb)