#include "KeyboardHandler.h"
#include "findNodeVisitor.h"

#include <osgViewer/Viewer>
#include <osgGA/UFOManipulator>
#include <osgGA/TrackballManipulator>
#include <osg/Vec3>
#include <osg/NodeVisitor>
#include <osg/Node>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Geode>
#include <osg/Projection>

#include <osg/Transform>
#include <osgSim/DOFTransform>
#include <osg/PositionAttitudeTransform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Switch>
#include <iostream>
using std::cout;

class tankDataType : public osg::Referenced
{
public:
   tankDataType(osg::Node*n); 
   void updateTurretRotation();
   void updateGunElevation();
   void setRotate(bool r, bool l) {rotate = r;left = l;}
protected:
   osgSim::DOFTransform* tankTurretNode;
   osgSim::DOFTransform* tankGunNode;
   double rotation;
   double elevation;
   bool rotate;
   bool up;
   bool left;
};

void tankDataType::updateTurretRotation()
{   
   if (rotate)
   {
      if (left) rotation += 0.001;
      else rotation -= 0.001;
      tankTurretNode->setCurrentHPR( osg::Vec3(rotation,0,0) );
   }
}


void tankDataType::updateGunElevation()
{
   if (elevation > .5)
      up = false;
   if (elevation < .01)
      up = true;
   if (up) 
     elevation += 0.0001;
   else
     elevation -= 0.0001;
   tankGunNode->setCurrentHPR( osg::Vec3(0,elevation,0) );
}


tankDataType::tankDataType(osg::Node* n)
{
   rotation = 0;
   elevation = 0;
   rotate = false;

   findNodeVisitor findNode("turret"); 
   n->accept(findNode);
   tankTurretNode = 
      dynamic_cast <osgSim::DOFTransform*> (findNode.getFirst());

   findNodeVisitor findGun("gun"); 
   n->accept(findGun);
   tankGunNode = 
      dynamic_cast< osgSim::DOFTransform*> (findGun.getFirst());
}

tankDataType* tankOneData = NULL;

void stopTurretRotation()
{
   tankOneData->setRotate(false,true);
}

void rotateTurretLeft()
{
   cout << "j";
   tankOneData->setRotate(true,true);
}
void rotateTurretRight()
{
   cout << "k";
   tankOneData->setRotate(true,false);
}
void toggleAA()
{
   std::cout << "code to toggle AntiAliasing goes here" << std::endl;
}
class tankNodeCallback : public osg::NodeCallback 
{
public:
   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {
      osg::ref_ptr<tankDataType> tankData = 
         dynamic_cast<tankDataType*> (node->getUserData() );
      if(tankData != NULL)
      {
         tankData->updateTurretRotation();
         tankData->updateGunElevation();
      }
      traverse(node, nv); 
   }
};
int main( int argc, char **argv )
{
   osg::Node* tankNode = NULL;
   osg::Group* root = new osg::Group();
   osgViewer::Viewer viewer;

   tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   root->addChild(tankNode);

   tankOneData = new tankDataType(tankNode);

   tankNode->setUserData( tankOneData );  // attach tankOneData to node
   tankNode->setUpdateCallback( new tankNodeCallback );
   //float maxr = tankNode->getMaxRange(0);
   // add a viewport to the viewer and attach the scene graph.
   viewer.setSceneData( root );

   keyboardEventHandler* keh = new keyboardEventHandler();

   keh->addFunction('a',toggleAA);

   keh->addFunction('k',keyboardEventHandler::KEY_DOWN,rotateTurretRight);
   keh->addFunction('k',keyboardEventHandler::KEY_UP,stopTurretRotation);
   keh->addFunction('j',rotateTurretLeft);
   keh->addFunction('j',keyboardEventHandler::KEY_UP,stopTurretRotation);       

   viewer.setCameraManipulator(new osgGA::TrackballManipulator());
   viewer.setSceneData( root );
   //viewer.getEventHandlers().push_front(keh); 
   viewer.addEventHandler(keh); 
   viewer.realize();

   while( !viewer.done() )
   {

      // fire off the cull and draw traversals of the scene.
      viewer.frame();

   }
   return 0;
}
