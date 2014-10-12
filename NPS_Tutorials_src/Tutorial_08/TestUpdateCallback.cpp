#include <osg/Group>
#include <osg/Node>
#include <osgDB/ReadFile> 
#include <osgViewer/Viewer>
#include <osgSim/DOFTransform>
#include <osgSim/MultiSwitch>
#include <osgGA/TrackballManipulator>
#include "findNodeVisitor.h"

class tankDataType : public osg::Referenced
{
public:
   tankDataType(osg::Node*n); 
   void updateTurretRotation();
   void updateGunElevation();
protected:
   osgSim::DOFTransform* tankTurretNode;
   osgSim::DOFTransform* tankGunNode;
   double rotation;
   double elevation;
   bool up;
};

void tankDataType::updateTurretRotation()
{   
   rotation += 0.0001;
   tankTurretNode->setCurrentHPR( osg::Vec3(rotation,0,0) );
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

   findNodeVisitor findNode("turret"); 
   n->accept(findNode);
   tankTurretNode = 
      dynamic_cast <osgSim::DOFTransform*> (findNode.getFirst());

   findNodeVisitor findGun("gun"); 
   n->accept(findGun);
   tankGunNode = 
      dynamic_cast< osgSim::DOFTransform*> (findGun.getFirst());
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

int main()
{
   osg::Node* tankNode = NULL;
   osg::Group* root = new osg::Group();
   osgViewer::Viewer viewer;

   tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   root->addChild(tankNode);

   tankDataType* tankData = new tankDataType(tankNode);

   tankNode->setUserData( tankData );
   tankNode->setUpdateCallback(new tankNodeCallback);

   viewer.setCameraManipulator(new osgGA::TrackballManipulator());
   viewer.setSceneData( root );
   viewer.realize();

   while( !viewer.done() )
   {
      viewer.frame();
   }
}
