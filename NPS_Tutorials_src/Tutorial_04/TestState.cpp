#include "stdafx.h"
//#include <osg/Node>
//#include <osg/Group>
//#include <osg/Geode>
//#include <osg/Geometry>
//#include <osg/Texture2D>
//#include <osgDB/ReadFile> 
//#include <osgViewer/Viewer>
//#include <osg/PositionAttitudeTransform>
//#include <osgGA/TrackballManipulator>
//#include <osg/StateSet>
//#include <osg/Texture2D>
//#include <osg/TexEnv>
//#include <osg/TexGen>
//#include <osg/ShapeDrawable>
#include <iostream>

namespace {
osg::Geode* createPyramid()
{
   osg::Geode* pyramidGeode = new osg::Geode();
   osg::Geometry* pyramidGeometry = new osg::Geometry();
   pyramidGeode->addDrawable(pyramidGeometry); 

   osg::Vec3Array* pyramidVertices = new osg::Vec3Array;
   pyramidVertices->push_back( osg::Vec3(0, 0, 0) ); // front left 
   pyramidVertices->push_back( osg::Vec3(2, 0, 0) ); // front right 
   pyramidVertices->push_back( osg::Vec3(2, 2, 0) ); // back right 
   pyramidVertices->push_back( osg::Vec3( 0,2, 0) ); // back left 
   pyramidVertices->push_back( osg::Vec3( 1, 1,2) ); // peak

   //Associate this set of vertices with the geometry associated with the 
   //geode we added to the scene.

   pyramidGeometry->setVertexArray( pyramidVertices );

   osg::TemplateIndexArray
      <unsigned int, osg::Array::UIntArrayType,4,4> *pyramidVertIndexArray;
   pyramidVertIndexArray = 
      new osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType,4,4>;
   
   //pyramidGeometry->setVertexIndices(pyramidVertIndexArray);

   osg::DrawElementsUInt* pyramidBase = 
      new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 3);
   pyramidBase->push_back(3);
   pyramidBase->push_back(2);
   pyramidBase->push_back(1);
   pyramidBase->push_back(0);
   pyramidGeometry->addPrimitiveSet(pyramidBase);

   osg::DrawElementsUInt* pyramidFaceOne = 
      new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
   pyramidFaceOne->push_back(0);
   pyramidFaceOne->push_back(1);
   pyramidFaceOne->push_back(4);
   pyramidGeometry->addPrimitiveSet(pyramidFaceOne);

   osg::DrawElementsUInt* pyramidFaceTwo = 
      new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
   pyramidFaceTwo->push_back(1);
   pyramidFaceTwo->push_back(2);
   pyramidFaceTwo->push_back(4);
   pyramidGeometry->addPrimitiveSet(pyramidFaceTwo);

   osg::DrawElementsUInt* pyramidFaceThree = 
      new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
   pyramidFaceThree->push_back(2);
   pyramidFaceThree->push_back(3);
   pyramidFaceThree->push_back(4);
   pyramidGeometry->addPrimitiveSet(pyramidFaceThree);

   osg::DrawElementsUInt* pyramidFaceFour = 
      new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
   pyramidFaceFour->push_back(3);
   pyramidFaceFour->push_back(0);
   pyramidFaceFour->push_back(4);
   pyramidGeometry->addPrimitiveSet(pyramidFaceFour);

   osg::Vec4Array* colors = new osg::Vec4Array;
   colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
   colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) ); //index 1 green
   colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) ); //index 2 blue
   colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) ); //index 3 white

   osg::TemplateIndexArray
      <unsigned int, osg::Array::UIntArrayType,4,4> *colorIndexArray;
   colorIndexArray = 
      new osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType,4,4>;
   colorIndexArray->push_back(0); // vertex 0 assigned color array element 0
   colorIndexArray->push_back(1); // vertex 1 assigned color array element 1
   colorIndexArray->push_back(2); // vertex 2 assigned color array element 2
   colorIndexArray->push_back(3); // vertex 3 assigned color array element 3
   colorIndexArray->push_back(0); // vertex 4 assigned color array element 0

   pyramidGeometry->setColorArray(colors);
// FIXME setColorIndices marked as depecated
   // pyramidGeometry->setColorIndices(colorIndexArray);
   pyramidGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

   osg::Vec2Array* texcoords = new osg::Vec2Array(5);
   (*texcoords)[0].set(0.00f,0.0f);
   (*texcoords)[1].set(0.25f,0.0f);
   (*texcoords)[2].set(0.50f,0.0f);
   (*texcoords)[3].set(0.75f,0.0f);
   (*texcoords)[4].set(0.50f,1.0f);

   pyramidGeometry->setTexCoordArray(0,texcoords);
   return pyramidGeode;
}

}

int main_TestState(int argc, char** argv)
{

   osgViewer::Viewer viewer;

   // Declare a group to act as root node of a scene:
   osg::Group* root = new osg::Group();

   // Declare a box class (derived from shape class) instance
   // This constructor takes an osg::Vec3 to define the center
   //  and a float to define the height, width and depth.
   //  (an overloaded constructor allows you to specify unique
   //   height, width and height values.)
   osg::Box* unitCube = new osg::Box( osg::Vec3(0,0,0), 1.0f);
   unitCube->setDataVariance(osg::Object::DYNAMIC);

   // Declare an instance of the shape drawable class and initialize 
   //  it with the unitCube shape we created above.
   //  This class is derived from 'drawable' so instances of this
   //  class can be added to Geode instances.
   osg::ShapeDrawable* unitCubeDrawable = new osg::ShapeDrawable(unitCube);

   // Declare a instance of the geode class: 
   osg::Geode* basicShapesGeode = new osg::Geode();

   // Add the unit cube drawable to the geode:
   basicShapesGeode->addDrawable(unitCubeDrawable);

   // Add the goede to the scene:
   root->addChild(basicShapesGeode);

   osg::Sphere* unitSphere = new osg::Sphere( osg::Vec3(0,0,0), 1.0);
   osg::ShapeDrawable* unitSphereDrawable = new osg::ShapeDrawable(unitSphere);
   unitSphereDrawable->setColor( osg::Vec4(0.1, 0.1, 0.1, 0.1) );

   osg::PositionAttitudeTransform* unitSphereXForm = 
      new osg::PositionAttitudeTransform();
   unitSphereXForm->setPosition(osg::Vec3(3.0,0,0));

   osg::Geode* unitSphereGeode = new osg::Geode();
   root->addChild(unitSphereXForm);
   unitSphereXForm->addChild(unitSphereGeode);
   unitSphereGeode->addDrawable(unitSphereDrawable);

   osg::Geode* pyramidGeode = createPyramid();
   
   osg::PositionAttitudeTransform* pyramidXForm = 
      new osg::PositionAttitudeTransform();
   pyramidXForm->setPosition(osg::Vec3(0,-3.0,0));
   root->addChild(pyramidXForm);
   pyramidXForm->addChild(pyramidGeode);

   osg::Texture2D* KLN89FaceTexture = new osg::Texture2D;
   // protect from being optimized away as static state:
   KLN89FaceTexture->setDataVariance(osg::Object::DYNAMIC); 
   osg::Image* klnFace = osgDB::readImageFile("../NPS_Data/Textures/KLN89FaceB.tga");
   if (!klnFace)
   {
      std::cout << " couldn't find texture, quitting." << std::endl;
      return -1;
   }
   KLN89FaceTexture->setImage(klnFace);

   // Declare a state set for 'BLEND' texture mode
   osg::StateSet* blendStateSet = new osg::StateSet();

   // Declare a TexEnv instance, set the mode to 'BLEND'
   osg::TexEnv* blendTexEnv = new osg::TexEnv;
   blendTexEnv->setMode(osg::TexEnv::BLEND);

   // Turn the attribute of texture 0 'ON'
   blendStateSet->setTextureAttributeAndModes(0,KLN89FaceTexture,osg::StateAttribute::ON);
   // Set the texture texture environment for texture 0 to the 
   //  texture envirnoment we declared above:
   blendStateSet->setTextureAttribute(0,blendTexEnv);

   osg::StateSet* decalStateSet = new osg::StateSet();
   osg::TexEnv* decalTexEnv = new osg::TexEnv();
   decalTexEnv->setMode(osg::TexEnv::DECAL);

   decalStateSet->setTextureAttributeAndModes(0,KLN89FaceTexture,osg::StateAttribute::ON);
   decalStateSet->setTextureAttribute(0,decalTexEnv);

   root->setStateSet(blendStateSet);
   unitSphereGeode->setStateSet(decalStateSet);

   // OSG1 viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
   viewer.setSceneData( root );
   viewer.setCameraManipulator(new osgGA::TrackballManipulator());
   viewer.realize();

   while( !viewer.done() )
   {
      viewer.frame();
   } 
   return 0;
}
