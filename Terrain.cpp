#include "stdafx.h"
#include "Terrain.h"

#include "find_node_visitor.h" 
#include "find_tex_visitor.h"
#include "find_animation.h"

#include "creators.h"
#include <windows.h>
#include "shaders.h"
#include "ct_visitor.h"

#include "materials_visitor.h"
#include "pugixml.hpp"
#include "high_res_timer.h"

#include <osgwTools/AbsoluteModelTransform.h>
#include "bi/BulletInterface.h"
#include "bi/BulletMTInterface.h"
#include "bi/RigidMTUpdater.h"



namespace avTerrain
{
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 


Terrain::Terrain (osg::Group* sceneRoot)
    : _sceneRoot(sceneRoot)
{
    sceneRoot->addChild(this);
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
        scene_name = "sheremetyevo.open.osgb";//"sheremetyevo.osgb";// "sheremetyevo.open.osgb";   
        mat_file_name = "sheremetyevo.open.dae"; 
    }
    else
    {
        scene_name = "adler.osgb";  
        mat_file_name = "adler.open.dae"; 
    }


    osg::Node* scene = osgDB::readNodeFile(name + "_dae/"+ scene_name);  

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

    MaterialVisitor mv ( nl, creators::createMaterial,creators::computeAttributes,mat::reader::read(name + "_dae/"+mat_file_name));
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

    auto ret_array  = creators::createMovingModel(center,radius*0.8f);

    osg::Node* movingModel = ret_array[0];

    addChild(baseModel);
    baseModel->setName("baseModel");
    
    addChild(movingModel);
    movingModel->setName("movingModel");

    OSG_WARN << "Время загрузки копирования моделей: " << _hr_timer.get_delta() << "\n";

    // osgDB::writeNodeFile(*movingModel,"test_osg_struct.osgt");

}





}