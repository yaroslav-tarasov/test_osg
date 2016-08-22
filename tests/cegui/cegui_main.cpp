/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/
#include "stdafx.h"

#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>

#include <cegui/CEGUI.h>
#include "CEGUIDrawable.h"

#include "../common/CommonFunctions"

int main_cegui( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setCullingActive( false );
    geode->addDrawable( new avGui::CEGUIDrawable );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 800, 0, 600);
    hudCamera->setAllowEventFocus( true );
    hudCamera->addChild( geode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFile("cow.osg") );
    root->addChild( hudCamera.get() );
    
    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new avGui::CEGUIEventHandler(hudCamera.get()) );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}

AUTO_REG(main_cegui) 