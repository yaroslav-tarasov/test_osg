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
   viewer.setCameraManipulator(Tman);   viewer.setSceneData( root );
   viewer.realize();

   osg::Matrixd myCameraMatrix;
   osg::Matrixd cameraRotation;
   osg::Matrixd cameraTrans;
   cameraRotation.makeRotate(
      osg::DegreesToRadians(-20.0), osg::Vec3(0,1,0), // roll
      osg::DegreesToRadians(-15.0), osg::Vec3(1,0,0) , // pitch
      osg::DegreesToRadians( 10.0), osg::Vec3(0,0,1) ); // heading

   // 60 meters behind and 7 meters above the tank model (calculated relatively)
   cameraTrans.makeTranslate( 10,-50,15 );

   myCameraMatrix = cameraRotation * cameraTrans;

   keyboardEventHandler* keh = new keyboardEventHandler();
   keh->addFunction('v',toggleView);
   viewer.addEventHandler(keh); 

   while( !viewer.done() )
   {

      if (manuallyPlaceCamera)
      {
         osg::Matrixd i = myCameraMatrix.inverse(myCameraMatrix);
         Tman->setByInverseMatrix( 
            osg::Matrix(i.ptr() )
            * osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ) );
      }

      viewer.frame();
   }
}
