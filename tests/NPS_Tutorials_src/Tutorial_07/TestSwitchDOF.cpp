#include <osg/Group>
#include <osgDB/ReadFile> 
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osgSim/DOFTransform>
#include <osgSim/MultiSwitch>
#include "findNodeVisitor.h"
#include <osgGA/TrackballManipulator>

#include <osgText/Font>
#include <osgText/Text>

// helper function to add a text label below a 'group'
bool addTextLabel(osg::Group* g, std::string s);

int main()
{
   // Declare a group for the root of our tree and 
   //  three groups to contain individual tank models
   osg::Group* root = new osg::Group();
   osg::Group* tankOneGroup = NULL;
   osg::Group* tankTwoGroup = NULL;
   osg::Group* tankThreeGroup = NULL;

   osgViewer::Viewer viewer;

   // Load the models from the same file
   tankOneGroup = dynamic_cast<osg::Group*> 
      (osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt"));
   tankTwoGroup = dynamic_cast<osg::Group*>
      (osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt"));
   tankThreeGroup = dynamic_cast<osg::Group*> 
      (osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt"));
   
   // findNodeVisitor will turn osgSim::DOFTransform nodes 
   // animation field to false
   tankOneGroup->accept( *(new findNodeVisitor("")) );

   // quit if we didn't successfully load the models
   if (! (tankOneGroup && tankTwoGroup && tankThreeGroup) )
   {
      std::cout << "could not load tank models" << std::endl;
      return -1;
   }

   // add the first tank as a child of the root node
   root->addChild(tankOneGroup);

   // declare a transform for positioning the second tank
   osg::PositionAttitudeTransform* tankTwoPAT = 
      new osg::PositionAttitudeTransform();
   // move the second tank five units right, five units forward
   tankTwoPAT->setPosition( osg::Vec3(5,5,0) );
   // add the tank as a child of its transform to the scene
   root->addChild(tankTwoPAT);
   tankTwoPAT->addChild(tankTwoGroup);

   // declare a transform for positioning the third tank
   osg::PositionAttitudeTransform* tankThreePAT = 
      new osg::PositionAttitudeTransform();
   // move the third tank ten units right
   tankThreePAT->setPosition( osg::Vec3(10,0,0) );
   // rotate the tank model 22.5 degrees to the left
   //  (to demonstrate that rotation of the turret will be 
   //   relative to the tank's heading)
   tankThreePAT->setAttitude( osg::Quat(3.14159/8.0, osg::Vec3(0,0,1) ));
   // add the tank as a child of its transform to the scene
   root->addChild(tankThreePAT);
   tankThreePAT->addChild(tankThreeGroup);

   // Declare an instance of 'findNodeVisitor' class and set its
   // searchForName string equal to "sw1"
   findNodeVisitor findNode("sw1"); 

   // Initiate traversal of this findNodeVisitor instance starting
   // from tankTwoGroup, searching all its children.  Build a list
   // of nodes whose names matched the searchForName string above.
   tankTwoGroup->accept(findNode);

   // Declare a switch type and assign it to the first node
   // in our list of matching nodes.
   osgSim::MultiSwitch* tankSwitch = NULL; 
   tankSwitch = dynamic_cast <osgSim::MultiSwitch*> (findNode.getFirst());

   // make sure it's a valid handle. If it is, set the first (only) 
   // multi-switch:
   if (tankSwitch)
   {
      //tankSwitch->setSingleChildOn(0,false); // good model
      tankSwitch->setSingleChildOn(0,true); // bad model
   }

   // Declare an instance of 'findNodeVisitor'; set the name to search
   // for to "turret"
   findNodeVisitor findTurretNode("turret");
   // Initiate a traversal starting from the subtree that represents 
   // the third tank model we loaded.
   tankThreeGroup->accept(findTurretNode);

   // Make sure we found a node and it's the correct type 
   osgSim::DOFTransform * turretDOF = 
      dynamic_cast<osgSim::DOFTransform *> (findTurretNode.getFirst()) ;

   // if it's a valid DOF node, set the heading of the turret
   // to 22.5 degrees right relative to the tank's heading.
   if (turretDOF)
   {
      turretDOF->setCurrentHPR( osg::Vec3(-3.14159/4.0,0.0,0.0) );
   }

   // Declare an instance of 'findNodeVisitor'; set the name to search
   // for to "gun"
   findNodeVisitor findGunNode("gun");
   // Initiate a traversal from the subtree that represents the third
   // tank model we loaded.
   tankThreeGroup->accept(findGunNode);

   // Make sure we found a node and it's the correct type
   osgSim::DOFTransform * gunDOF = 
      dynamic_cast<osgSim::DOFTransform *> (findGunNode.getFirst()) ;

   // If the node we found is a valid  DOF transform, set the pitch
   // to 22.5 degrees up relative to the turret's pitch.
   if (gunDOF)
   {
      gunDOF->setCurrentHPR( osg::Vec3(0.0,3.14159/8.0,0.0) );
   }

   addTextLabel(tankOneGroup,"Original Tank");
   addTextLabel(tankTwoGroup,"Damaged State");
   addTextLabel(tankThreeGroup,"Articulated");

   viewer.setSceneData( root );
   viewer.setCameraManipulator(new osgGA::TrackballManipulator());
   viewer.realize();

   while( !viewer.done() )
   {
      viewer.frame();
   }
}

bool addTextLabel(osg::Group* g, std::string s)
{
   if (!g)
   {
      return false;
   }
   osg::Geode* textLabelGeode = new osg::Geode();
   osgText::Text* textOne = new osgText::Text();
   g->addChild(textLabelGeode);
   textLabelGeode->addDrawable(textOne);

   textOne->setCharacterSize(1);
   textOne->setFont("../impact.ttf");
   textOne->setText(s);
   textOne->setAxisAlignment(osgText::Text::SCREEN);
   textOne->setColor( osg::Vec4(1,0,0,1.0f) );
   textOne->setPosition( osg::Vec3(2,1,-1) );
   //textOne->setDrawMode(osgText::Text::TEXT |
   //                          osgText::Text::ALIGNMENT | 
   //                             osgText::Text::BOUNDINGBOX);
   textOne->setAlignment(osgText::Text::CENTER_TOP);
   return true;
}
