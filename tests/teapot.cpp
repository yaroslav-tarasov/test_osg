#include "stdafx.h"
#include "teapot.h"



int main_teapot( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( new TeapotDrawable(1.0f) );
    
    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( root.get() );
    return viewer.run();
}

 AUTO_REG(main_teapot)