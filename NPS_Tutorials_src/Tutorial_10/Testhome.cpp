#include <osg/NodeCallback>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile> 
#include <osgGA/TrackballManipulator>
#include "KeyboardHandler.h"

bool manuallyPlaceCamera = false;

void toggleView()
{
   if (! manuallyPlaceCamera)
      manuallyPlaceCamera = true;
   else
      manuallyPlaceCamera = false;
}

int main()
{
   osg::Node* groundNode = NULL;
   osg::Node* tankNode = NULL;
   osg::Group* root = new osg::Group();
   osgViewer::Viewer viewer;
   osg::PositionAttitudeTransform* tankXform;

   groundNode = osgDB::readNodeFile("../NPS_Data/Models/JoeDirt/JoeDirt.flt");
   tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");

   // Create green Irish sky
   osg::ClearNode* backdrop = new osg::ClearNode;
   backdrop->setClearColor(osg::Vec4(0.0f,0.8f,0.0f,1.0f));
   root->addChild(backdrop);
   root->addChild(groundNode);

   tankXform = new osg::PositionAttitudeTransform();
   root->addChild(tankXform);
   tankXform->addChild(tankNode);

   tankXform->setPosition( osg::Vec3(10,10,8) );
   tankXform->setAttitude(
      osg::Quat(osg::DegreesToRadians(-45.0), osg::Vec3(0,0,1) ) );

   osgGA::TrackballManipulator *Tman = new osgGA::TrackballManipulator();
   viewer.setCameraManipulator(Tman);
   viewer.setSceneData( root );
   viewer.realize();

   osg::Vec3d  eye;
   osg::Vec3d  centre;
   osg::Vec3d  up;
   keyboardEventHandler* keh = new keyboardEventHandler();
   keh->addFunction('v',toggleView);
   viewer.addEventHandler(keh); 
   centre = osg::Vec3(10.0,10.0,10.0);
   up = osg::Vec3(0.0,0.0,1.0);
   eye = osg::Vec3 (10,0.0, 10.0);
   
   viewer.home();
   while( !viewer.done() )
   {

      if (manuallyPlaceCamera)
        {
        Tman -> setHomePosition(eye, centre, up, false);
        viewer.home();
        }
      viewer.frame();
   }


}
