#include "stdafx.h"
#include "creators.h"


// #define TEST_SHADOWS

namespace {
#define SOFT_SHADOW_MAP

creators::nodes_array_t createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique)
{
    osg::Vec3 center(0.0f,0.0f,300.0f);
    float radius = 600.0f;

    // osg::Group* root = new osg::Group;
    // Light.
    osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
    source->getLight()->setPosition(osg::Vec4(0, 0, 2, 0));
    source->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    source->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    //source->getLight()->setSpecular(osg::Vec4(0.8, 0.8, 0.8, 1));

    int shadowsize = 512*8;//1024;
#ifdef SOFT_SHADOW_MAP    
    osg::ref_ptr<osgShadow::SoftShadowMap> st = new osgShadow::SoftShadowMap;
    st->setTextureSize(osg::Vec2s(shadowsize, shadowsize));
    st->setTextureUnit(1);
    st->setJitteringScale(16);
    st->setSoftnessWidth(0.00005);
#else
    osg::ref_ptr<osgShadow::StandardShadowMap> st = new osgShadow::StandardShadowMap;
    //osg::ref_ptr<osgShadow::ShadowTexture> st = new osgShadow::ShadowTexture;
#endif

    //osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMap> st = new osgShadow::LightSpacePerspectiveShadowMap;

    // Scene
    osg::ref_ptr<osgShadow::ShadowedScene> root = new osgShadow::ShadowedScene(st.get());
    st->setLight(source->getLight());



    float baseHeight = 0.0f; //center.z();//-radius*0.5;
#ifdef TEST_REAL_SCENE 
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 

    osg::Node* adler = osgDB::readNodeFile("adler.osgb");
    osg::MatrixTransform* baseModel = new osg::MatrixTransform;
    baseModel->setMatrix(
        // osg::Matrix::translate(-bs.center())*  
        // osg::Matrix::scale(size,size,size)*
        osg::Matrix::rotate(quat0));

    baseModel->addChild(adler);
#else
    osg::Node* baseModel = creators::createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius*3);
#endif

    creators::nodes_array_t  ret_array;
    //auto ret_array  = createMovingModel(center,radius*0.8f);

    //osg::Node* movingModel = ret_array[0];
    osg::Node*  airplane = creators::loadAirplane();

    //osg::Group* test_model = new osg::Group;

    const bool add_planes = true;
    if (add_planes)
    {
        auto p_copy = //creators::loadBMAirplane(true);
        creators::applyBM(creators::loadAirplane(),"a_319");

        //effects::createShader(p_copy) ;

        const unsigned inst_num = 12;
        for (unsigned i = 0; i < inst_num; ++i)
        {
            float const angle = 2.0f * /*cg::pif*/osg::PI * i / inst_num, radius = 200.f;
            /*cg::point_3f*/ osg::Vec3 pos(radius * sin (angle), radius * cos(angle), 0.f);

            const osg::Quat quat(osg::inDegrees(-90.f), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(180.f * (i & 1)) - angle  , osg::Z_AXIS ); 

            osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(pos));
            positioned->setDataVariance(osg::Object::STATIC);

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
            rotated->setDataVariance(osg::Object::STATIC);

            positioned->addChild(rotated);
            rotated->addChild(p_copy);

            root->addChild(positioned);

        }
    }
 

    root->addChild(baseModel);
    root->addChild(airplane/*movingModel*/);
    root->addChild(source.get());


    ret_array[0] = root.release();

    return ret_array;
}

}

int main_shadows_2( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osg::DisplaySettings::instance()->setNumMultiSamples( 8 );

    osgViewer::Viewer viewer(arguments);

    // Set the clear color to black
    viewer.getCamera()->setClearColor(osg::Vec4(0,0,0,1));

    // tilt the scene so the default eye position is looking down on the model.
    osg::ref_ptr<osg::MatrixTransform> rootnode = new osg::MatrixTransform;
    //rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),1.0f,0.0f,0.0f));

    // Use a default camera manipulator
    osgGA::TrackballManipulator* manip = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(manip);
    // Initially, place the TrackballManipulator so it's in the center of the scene
    manip->setHomePosition(osg::Vec3(0,1000,1000), osg::Vec3(0,1,0), osg::Vec3(0,0,1));
    manip->home(0);

    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(rootnode->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);

    // auto model = osgDB::readNodeFile("an_124.dae");  /*a_319.3ds*/
    bool overlay = false;
    osgSim::OverlayNode::OverlayTechnique technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY;//  osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;

    // load the nodes from the commandline arguments.
    auto model_parts  = createModel(overlay, technique);

    osg::Node* model = model_parts[0];

    if(model == nullptr)
    {
        osg::notify(osg::WARN) << "Can't load " <<  "an_124.dae";
        return 1;
    }
    else
    {

#ifdef TEST_SHADOWS        
        // Light.
        osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
        source->getLight()->setPosition(osg::Vec4(4, 4, 4, 0));
        source->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
        source->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
        source->getLight()->setSpecular(osg::Vec4(0.8, 0.8, 0.8, 1));

        int shadowsize = 1024;//1024;
        osg::ref_ptr<osgShadow::SoftShadowMap> sm = new osgShadow::SoftShadowMap;
        sm->setTextureSize(osg::Vec2s(shadowsize, shadowsize));
        sm->setTextureUnit(1);
        sm->setJitteringScale(16);
        // Scene
        osg::ref_ptr<osgShadow::ShadowedScene> root = new osgShadow::ShadowedScene;
        root->setShadowTechnique(sm.get());
        sm->setLight(source.get());

        root->addChild(model);
        rootnode->asGroup()->addChild(source);
#endif
        rootnode->addChild(model);


        // run optimization over the scene graph
        //osgUtil::Optimizer optimzer;
        //optimzer.optimize(rootnode);

        // set the scene to render
        viewer.setSceneData(rootnode/*model*/);


        osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
        viewer.addEventHandler(statesetManipulator.get());



        return viewer.run();
    }

    return 1;
}