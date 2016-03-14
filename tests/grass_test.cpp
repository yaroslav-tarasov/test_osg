#include "stdafx.h"
#include "visitors/info_visitor.h"
#include "visitors/find_tex_visitor.h"
#include "creators.h"
#include "av/avTerrain/Terrain.h"


int main_grass_test( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");   

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
   
    osg::ref_ptr<osg::Node> sub_model = osgDB::readNodeFile( "glider.osg" );

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(osgDB::readImageFile("a_319_aeroflot.png"/*"Fieldstone.jpg"*/));

    creators::nodes_array_t plane = creators::loadAirplaneParts("a_319");
    
    std::string scene_name("minsk"); // "empty","adler" ,"sheremetyevo"
    
    osg::StateSet * pSceneSS = root->getOrCreateStateSet();
    osg::ref_ptr<osg::Uniform> _ambientUniform = new osg::Uniform("ambient", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_ambientUniform.get());

    osg::ref_ptr<osg::Uniform> _diffuseUniform = new osg::Uniform("diffuse", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_diffuseUniform.get());

    // FIXME TODO secular.a wanna rain
    osg::ref_ptr<osg::Uniform> _specularUniform = new osg::Uniform("specular", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_specularUniform.get());

    osg::ref_ptr<osg::Uniform> _lightDirUniform = new osg::Uniform("light_vec_view", osg::Vec4f(0.f, 0.f, 0.f, 1.f));
    pSceneSS->addUniform(_lightDirUniform.get());

    osg::ref_ptr<avTerrain::Terrain> _terrainNode =  new avTerrain::Terrain (root);
    _terrainNode->create(scene_name);
    
    root->addChild(_terrainNode);
    root->addChild(plane[0]);
    root->addChild(sub_model.get());

  
    // osgDB::writeNodeFile(*root,"tex_test_blank.osgt");
	
	// Set the clear color to black
    // viewer.getCamera()->setClearColor(osg::Vec4(1.0,0,0,1));

    viewer.apply(new osgViewer::SingleScreen(1));
    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}

AUTO_REG(main_grass_test)