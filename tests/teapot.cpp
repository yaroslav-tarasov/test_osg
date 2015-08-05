#include "stdafx.h"
#include "teapot.h"
#include "av/Grass.h"




int main_teapot( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
	osg::ref_ptr<osg::Group> root= new osg::Group;
	osg::ref_ptr<osg::Geode> teapot = new osg::Geode;
	teapot->addDrawable( new TeapotDrawable(1.0f) );
	root->addChild(teapot);

	root->addChild(new avTerrain::Grass());


    osgViewer::Viewer viewer(arguments);
		viewer.getCamera()->setCullingMode( osg::CullSettings::NO_CULLING );
	viewer.apply(new osgViewer::SingleScreen(0));

    viewer.setSceneData( root.get() );
    return viewer.run();
}

 AUTO_REG(main_teapot)