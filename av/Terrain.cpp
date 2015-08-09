#include "stdafx.h"

#include "Terrain.h"

#include "visitors/find_tex_visitor.h"
#include "visitors/find_animation.h"

#include "creators.h"
//#include <windows.h>
#include "shaders.h"
#include "visitors/ct_visitor.h"

#include "visitors/materials_visitor.h"
#include "pugixml.hpp"
#include "high_res_timer.h"
#include "utils/callbacks.h"

#include <osgwTools/AbsoluteModelTransform.h>
#include "phys/BulletInterface.h"
#include "phys/BulletMTInterface.h"
#include "phys/RigidMTUpdater.h"


#include "av/Grass.h"
#include "av/Grass2.h"

namespace avTerrain
{
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 


Terrain::Terrain (osg::Group* sceneRoot)
    : _sceneRoot(sceneRoot)
    , _lightsHandler(avScene::GlobalInfluence)
{
    sceneRoot->addChild(this);
    //
    // Callbacks
    //

    // callbacks setup
    setCullCallback(utils::makeNodeCallback(this, &Terrain::cull, true));
}

void  Terrain::create( std::string name )
{
    const   osg::Vec3 center(0.0f,0.0f,300.0f);
    const   float radius = 600.0f;
    high_res_timer                _hr_timer;
    osg::MatrixTransform*         baseModel = new osg::MatrixTransform;

    float baseHeight = 0.0f; 
    if(name != "empty" && !name.empty() )
    {

    std::string scene_name;
    std::string mat_file_name;

    if(name == "sheremetyevo")
    {
        scene_name =    "sheremetyevo.osgb";//"sheremetyevo.open.osgb";//"sheremetyevo.osgb"; 
        mat_file_name = "sheremetyevo.dae.mat.xml"; //"sheremetyevo.open.dae.mat.xml"; 
    }
    else if(name == "adler")
    {
        scene_name = "adler.osgb";  
        mat_file_name = "adler.open.dae.mat.xml"; 
    }
	else if(name == "minsk")
	{
        scene_name = "minsk.dae";  
        mat_file_name = "minsk.dae.mat.xml"; 
	}


    osg::Node* scene = osgDB::readNodeFile(name + "/"+ scene_name);  
    
    // Здесь был Минск
    //osg::PositionAttitudeTransform* pat = scene->asTransform()->asPositionAttitudeTransform();
    //pat->setAttitude( osg::Quat(osg::inDegrees(90.0),osg::X_AXIS,osg::inDegrees(/*-49.0 - 33.0*/0.0),osg::Y_AXIS,osg::inDegrees(0.0),osg::Z_AXIS));

    scene->setName("scene");

    auto lod3 =  findFirstNode(scene,"lod3");

    if(lod3) 
        lod3->setNodeMask(0); // Убираем нафиг Lod3 

    baseModel = new osg::MatrixTransform;
    baseModel->setMatrix(osg::Matrix::rotate(quat0));
    baseModel->addChild(scene);

    MaterialVisitor::namesList nl;
    nl.push_back("building");
    nl.push_back("tree");
    nl.push_back("ground"); 
    nl.push_back("concrete");
    nl.push_back("mountain");
    nl.push_back("sea");
    nl.push_back("railing");
    nl.push_back("panorama");
    nl.push_back("plane");

    MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,name,sp::_2,sp::_3),creators::computeAttributes,mat::reader::read(cfg().path.data + "/areas/" + name + "/"+mat_file_name));
    scene->accept(mv);

    // All solid objects
    osg::StateSet * pCommonStateSet = scene->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");
    pCommonStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    pCommonStateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

    // Scene 
    //Add backface culling to the whole bunch
    //  osg::StateSet * pSS = adler->getOrCreateStateSet();
    //pCommonStateSet->setNestRenderBins(false);
    //pCommonStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    //// disable alpha writes for whole bunch
    //pCommonStateSet->setAttribute(new osg::ColorMask(true, true, true, false));
    
   }
   else
   {  
       baseModel->addChild( creators::createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius*3));
   }
    baseModel->setDataVariance(osg::Object::STATIC);

    OSG_WARN << "Время загрузки сцены: " << _hr_timer.get_delta() << "\n";

    addChild(baseModel);
    baseModel->setName("baseModel");
	
    Grass* grass = new Grass();
    grass->setWindFactor(1.0);
    addChild(grass);
    _grass = grass;

	blender::Grass* grass2 = new blender::Grass();
	addChild(grass2);

#if 0
    auto ret_array  = creators::createMovingModel(center,radius*0.8f);
    osg::Node* movingModel = ret_array[0];
    addChild(movingModel);
    movingModel->setName("movingModel");
#endif

    OSG_WARN << "Время загрузки копирования моделей: " << _hr_timer.get_delta() << "\n";

    // osgDB::writeNodeFile(*movingModel,"test_osg_struct.osgt");

}

// cull method
void Terrain::cull( osg::NodeVisitor * pNV )
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
    avAssert(pCV);

    // cull down
    _lightsHandler.onCullBegin(pCV);
    pNV->traverse(*this);
    _lightsHandler.onCullEnd(pCV);
}

void Terrain::setGrassMapFactor(float val)
{
	if(_grass)
        dynamic_cast<Grass*>(_grass.get())->setGrassMapFactor(val);
}

}
