#include "stdafx.h"
#include "creators.h"
#include "av/shaders.h"
#include "av/avUtils/animation_handler.h"


static const std::string animationName("Default");


int main_anim_test( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    //osg::DisplaySettings::instance()->setNumMultiSamples( 8 );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;


/////////////////////////////////////////////////////////////////////////    
    //osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( "./data/models/tu_154/tu_154.dae" );
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( "./data/models/tu_134/tu_134.dae" );
//    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( "./data/models/su_27/su_27.dae" );
//    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( "./data/models/a_319/a_319.dae" );
    osg::Node* lod3 =  findFirstNode(model,"Lod3"); 
    if(lod3)
        lod3->setNodeMask(0);

    root->addChild(model.get());

    osgViewer::Viewer viewer(arguments);
    //viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
    //viewer.getCamera()->setNearFarRatio(0.00001f);

    viewer.apply(new osgViewer::SingleScreen(1));

    viewer.setSceneData( root.get() );

#if 1
    // Use a default camera manipulator
    osgGA::TrackballManipulator* manip = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(manip);
    viewer.addEventHandler(new AnimationHandler(model,animationName
            ,nullptr
            ,nullptr
            ,nullptr
    ));
#endif

    viewer.addEventHandler(new osgViewer::StatsHandler);

#if 0
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
#endif
    


    return viewer.run();
}

AUTO_REG(main_anim_test)
