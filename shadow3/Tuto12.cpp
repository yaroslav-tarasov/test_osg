#include "stdafx.h" 
#include "../creators.h"


using namespace osg;

int main_shadows_3(int argc, char** argv)
{
	//Declaration of the objects that will form our scene
	osgViewer::Viewer viewer;
	ref_ptr<Group> scene (new Group);
	ref_ptr<Geode> objectGeode (new Geode);
	ref_ptr<Geode> terrainGeode (new Geode);
	ref_ptr<Geode> lightMarkerGeode (new Geode);
	
	//Shadow stuff!!!	
	ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
        shadowedScene->setShadowTechnique(sm.get());

	//Main light source 
	Vec3 lightPosition(0,0,3); 
	LightSource* ls = new LightSource;
  	ls->getLight()->setPosition(Vec4(lightPosition,1));
	ls->getLight()->setAmbient(Vec4(0.2,0.2,0.2,1.0));
        ls->getLight()->setDiffuse(Vec4(0.6,0.6,0.6,1.0));

	
	
	shadowedScene->addChild(scene.get());
	shadowedScene->addChild(ls);
	shadowedScene->addChild(lightMarkerGeode.get());
	//Next we define the material property of our objects 
	// material
    ref_ptr<Material> matirial = new Material;
    matirial->setColorMode(Material::DIFFUSE);
    matirial->setAmbient(Material::FRONT_AND_BACK, Vec4(0, 0, 0, 1));
    matirial->setSpecular(Material::FRONT_AND_BACK, Vec4(1, 1, 1, 1));
    matirial->setShininess(Material::FRONT_AND_BACK, 64);
    scene->getOrCreateStateSet()->setAttributeAndModes(matirial.get(), StateAttribute::ON);

	//Adding the terrain and object nodes to the root node
	scene->addChild(objectGeode.get());
	scene->addChild(terrainGeode.get());

	
	//The terrain first, a flatten box
	terrainGeode->addDrawable(new ShapeDrawable(new Box(Vec3f(),5,7,0.05f)));
	terrainGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::Texture2D(osgDB::readImageFile("terrain.jpg")));

	//Now we can define our world made of several objects  
	
	//The capsule
	objectGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, new Texture2D(osgDB::readImageFile("Fieldstone.jpg"))); // Fieldstone.jpg
	objectGeode->addDrawable(new ShapeDrawable(new Capsule(Vec3(1,-1,1),0.3f,0.5f)));
    objectGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, new Texture2D(osgDB::readImageFile("foo.jpg")));

	//The box
	objectGeode->addDrawable(new ShapeDrawable(new Box(Vec3(-1,1,1),1,1,1)));
	
	//The sphere
	objectGeode->addDrawable(new ShapeDrawable(new Sphere(Vec3(1,2,1),0.5f)));
	
	//objectGeode->addDrawable(new ShapeDrawable(new Sphere(Vec3(0,0,4.5f),0.1f)));

	//And finally the light marker: a small sphere
	lightMarkerGeode->addDrawable(new ShapeDrawable(new Sphere(lightPosition+osg::Vec3(0,0,0.5f),0.1f)));
    
    
    const osg::Quat quat(osg::inDegrees(90.f), osg::X_AXIS,                      
        osg::inDegrees(0.f) , osg::Y_AXIS,
        osg::inDegrees(0.f) , osg::Z_AXIS ); 

    auto node = osgDB::readNodeFile("a_319.x"); // creators::loadAirplane();
    node->getOrCreateStateSet()->setTextureAttributeAndModes(0, new Texture2D(osgDB::readImageFile("Fieldstone.jpg")));
    osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::scale(osg::Vec3f(0.1f,0.1f,0.1f)));
    osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
    positioned->addChild(rotated);
    rotated->addChild(node);

    scene->addChild(positioned);

	viewer.setSceneData( shadowedScene.get() );
	
	//Stats Event Handler s key
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the state manipulator
    	viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

	//Windows size handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	
	return (viewer.run());
}


