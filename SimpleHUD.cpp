
#include "stdafx.h"
#include <osg/PositionAttitudeTransform>

#include <osgText/Font>
#include <osgText/Text>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Geometry>

int main_hud( int argc, char** argv )
{
   osg::Group* root = NULL; 
   osg::Node* tankNode = NULL; 
   osg::Node* terrainNode = NULL;
   osg::PositionAttitudeTransform* tankXform;
   osg::Vec3 tankPosit; 
   osg::Geode* HUDGeode = new osg::Geode();
   osgText::Text* textOne = new osgText::Text();
   osgText::Text* tankLabel = new osgText::Text();
   osg::Projection* HUDProjectionMatrix = new osg::Projection;
   osgViewer::Viewer viewer;

   root = new osg::Group();
   tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   terrainNode = osgDB::readNodeFile("../NPS_Data/Models/JoeDirt/JoeDirt.flt");

   tankXform = new osg::PositionAttitudeTransform();
   tankPosit.set(5,5,8);
   tankXform->setPosition( tankPosit ); 
   root->addChild(terrainNode);
   root->addChild(tankXform);
   tankXform->addChild(tankNode);

   HUDProjectionMatrix->setMatrix(osg::Matrix::ortho2D(0,1024,0,768));

   osg::MatrixTransform* HUDModelViewMatrix = new osg::MatrixTransform;
   HUDModelViewMatrix->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   HUDModelViewMatrix->setMatrix(osg::Matrix::identity());

   root->addChild(HUDProjectionMatrix);
   HUDProjectionMatrix->addChild(HUDModelViewMatrix);
   HUDModelViewMatrix->addChild( HUDGeode );
   HUDGeode->addDrawable( textOne );

   textOne->setCharacterSize(25);
   textOne->setFont("../impact.ttf");
   textOne->setText("Not so good");
   textOne->setAxisAlignment(osgText::Text::SCREEN);
   textOne->setPosition( osg::Vec3(242,165,1) );
   textOne->setColor( osg::Vec4(1,0,0,1.0f) );

   tankLabel->setCharacterSize(2);
   tankLabel->setFont("../arial.ttf");
   tankLabel->setText("Tank #1");
   tankLabel->setAxisAlignment(osgText::Text::SCREEN);
   tankLabel->setDrawMode(osgText::Text::TEXT |
                             osgText::Text::ALIGNMENT | 
                                osgText::Text::BOUNDINGBOX);

   tankLabel->setAlignment(osgText::Text::CENTER_TOP);
   tankLabel->setPosition( osg::Vec3(0,0,4) );
   tankLabel->setColor( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );

   osg::Geode* tankLabelGeode = new osg::Geode();
   tankLabelGeode->addDrawable(tankLabel);
   tankXform->addChild(tankLabelGeode);

   osg::Geometry* HUDBackgroundGeometry = new osg::Geometry();

   osg::Vec3Array* HUDBackgroundVertices = new osg::Vec3Array;
   HUDBackgroundVertices->push_back( osg::Vec3(   0,  0,.8) );
   HUDBackgroundVertices->push_back( osg::Vec3(1024,  0,.8) );
   HUDBackgroundVertices->push_back( osg::Vec3(1024,200,.8) );
   HUDBackgroundVertices->push_back( osg::Vec3(   0,200,.8) );

   osg::DrawElementsUInt* HUDBackgroundIndices = 
      new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
   HUDBackgroundIndices->push_back(0);
   HUDBackgroundIndices->push_back(1);
   HUDBackgroundIndices->push_back(2);
   HUDBackgroundIndices->push_back(3);

   osg::Vec4Array* HUDcolors = new osg::Vec4Array;
   HUDcolors->push_back(osg::Vec4(0.8f,0.8f,0.8f,0.5f));

   osg::Vec2Array* texcoords = new osg::Vec2Array(4);
   (*texcoords)[0].set(0.0f,0.0f);
   (*texcoords)[1].set(1.0f,0.0f);
   (*texcoords)[2].set(1.0f,1.0f);
   (*texcoords)[3].set(0.0f,1.0f);
   HUDBackgroundGeometry->setTexCoordArray(0,texcoords);

   // set up the texture state.    
   osg::Texture2D* HUDTexture = new osg::Texture2D;
   // protect from being optimized away as static state.
   HUDTexture->setDataVariance(osg::Object::DYNAMIC); 
   HUDTexture->setImage(osgDB::readImageFile("../NPS_Data/Textures/sky.bmp"));

   osg::Vec3Array* HUDnormals = new osg::Vec3Array;
   HUDnormals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
   HUDBackgroundGeometry->setNormalArray(HUDnormals);
   HUDBackgroundGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

   HUDGeode->addDrawable(HUDBackgroundGeometry);
   HUDBackgroundGeometry->addPrimitiveSet(HUDBackgroundIndices);
   HUDBackgroundGeometry->setVertexArray(HUDBackgroundVertices);
   HUDBackgroundGeometry->setColorArray(HUDcolors);
   HUDBackgroundGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

   osg::StateSet* HUDStateSet = new osg::StateSet(); 
   HUDGeode->setStateSet(HUDStateSet);
   HUDStateSet->setRenderBinDetails( 11, "DepthSortedBin");
   HUDStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);
   HUDStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
   HUDStateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
   HUDStateSet->setTextureAttributeAndModes(0,HUDTexture,osg::StateAttribute::ON);
   osgUtil::RenderBin* HUDBin = 
      new osgUtil::RenderBin(osgUtil::RenderBin::SORT_BACK_TO_FRONT);
 
   //viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
   viewer.setSceneData( root );
   viewer.setCameraManipulator(new osgGA::TrackballManipulator());

   viewer.realize();

   while( !viewer.done() )
   {
      viewer.frame();
   }

   return 0;
}
