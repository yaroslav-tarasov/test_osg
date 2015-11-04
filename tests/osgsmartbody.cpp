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

#include <map>

class OSGSmartBodyListener : public SmartBody::SBSceneListener
{
public:
	OSGSmartBodyListener(osg::Group* sceneMgr, std::map<std::string, int>* characterMap);
	~OSGSmartBodyListener();

	virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );	
	virtual void OnCharacterDelete( const std::string & name );
	virtual void OnCharacterChanged( const std::string& name );		 
	virtual void OnLogMessage( const std::string & message );


private:
	osg::Group* mSceneMgr;
	std::map<std::string, int>* map;
	int id;
};

OSGSmartBodyListener::OSGSmartBodyListener(osg::Group* sceneMgr, std::map<std::string, int>* characterMap)
{
	mSceneMgr = sceneMgr;
	map = characterMap;
	id = 1;
}

OSGSmartBodyListener::~OSGSmartBodyListener()
{
}

void OSGSmartBodyListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
#if 0	
	std::stringstream strstr;
	strstr << "../irrlicht-1.8.1/media/" << objectClass<< ".mesh";
	std::cout << "Getting mesh " << strstr.str() << std::endl;

	irr::scene::ISkinnedMesh* skinnedMesh = (irr::scene::ISkinnedMesh*)mSceneMgr->getMesh(strstr.str().c_str());

	if (!skinnedMesh)
	{
		LOG("Cannot find mesh named '%s", objectClass.c_str());
		return;
	}

	irr::scene::IAnimatedMeshSceneNode* node = mSceneMgr->addAnimatedMeshSceneNode( skinnedMesh, NULL, id );

	//must set to allow manual joint control
	node->setJointMode(irr::scene::EJUOR_CONTROL);

	irr::scene::IAnimatedMesh* animMesh  = node->getMesh();
	irr::scene::E_ANIMATED_MESH_TYPE type = animMesh->getMeshType();

	irr::core::array<irr::scene::ISkinnedMesh::SJoint*> jointssss  = skinnedMesh->getAllJoints();


	for(irr::u32 i = 1; i < jointssss.size(); i++)
	{
		//Clear all joint animation keys - model takes less space on disk
		jointssss[i]->PositionKeys.clear();
		jointssss[i]->RotationKeys.clear();
		jointssss[i]->ScaleKeys.clear();
	}



	(*map)[name] = id;
	id++;

	//set texture
	std::string textures[9];
	textures[0] = "../irrlicht-1.8.1/media/sinbad_body.tga";
	textures[1] = "../irrlicht-1.8.1/media/sinbad_body.tga";
	textures[2] = "../irrlicht-1.8.1/media/sinbad_clothes.tga";
	textures[3] = "../irrlicht-1.8.1/media/sinbad_body.tga";
	textures[4] = "../irrlicht-1.8.1/media/sinbad_sword.tga";
	textures[5] = "../irrlicht-1.8.1/media/sinbad_clothes.tga";
	textures[6] = "../irrlicht-1.8.1/media/sinbad_clothes.tga";
	textures[7] = "../irrlicht-1.8.1/media/sinbad_clothes.tga";
	textures[8] = "../irrlicht-1.8.1/media/irrlicht2_dn.jpg";
	node->getMaterial(8).setTexture(0,mSceneMgr->getVideoDriver()->getTexture("../irrlicht-1.8.1/media/irrlicht2_dn.jpg"));
	for (int t = 0; t < 8; t++)
	{
		std::cout << "Attempting to retrieve " << textures[t] << std::endl;
		node->getMaterial(t).setTexture(0,mSceneMgr->getVideoDriver()->getTexture(textures[t].c_str()));
	}

	node->setPosition(irr::core::vector3df(0,-80,0));


	node->addShadowVolumeSceneNode();

	//mSceneMgr->setShadowColor(irr::video::SColor(150,0,0,0));

	node->setScale(irr::core::vector3df(10,10,10));
	node->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS , true);
#endif

}

void OSGSmartBodyListener::OnCharacterDelete( const std::string & name )
{
}

void OSGSmartBodyListener::OnCharacterChanged( const std::string& name )
{

}

void OSGSmartBodyListener::OnLogMessage( const std::string & message )
{
#ifdef WIN32
	// LOG(message.c_str());
#endif
}

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
		
		int numCharacters = scene->getNumCharacters();
		if (numCharacters == 0)
			return true;

		sim->updateTimer();
        viewer.frame();
    }
    sim->stop();
    return 0;
}


 AUTO_REG(main_sb)