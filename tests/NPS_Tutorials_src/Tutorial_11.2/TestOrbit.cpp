#include <osg/NodeCallback>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile> 
#include <osgGA/TrackballManipulator>
#include "KeyboardHandler.h"

class orbit : public osg::NodeCallback 
{
public:
   orbit(): heading(M_PI/2.0) {}  

   osg::Matrix getWCMatrix(){return worldCoordMatrix;} 

   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {
      osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
      if( tx != NULL )
      {
         heading += M_PI/1800.0;
         osg::Matrixd orbitRotation;
         orbitRotation.makeRotate(  
            osg::DegreesToRadians(-10.0), osg::Vec3(0,1,0),  // roll
            osg::DegreesToRadians(-20.0), osg::Vec3(1,0,0) , // pitch
            heading, osg::Vec3(0, 0, 1) );                   // heading
         osg::Matrixd orbitTranslation;
         orbitTranslation.makeTranslate( 0,-40, 4 );
         tx->setMatrix ( orbitTranslation * orbitRotation);
         worldCoordMatrix = osg::computeWorldToLocal( nv->getNodePath() );
      }

      traverse(node, nv);
   }
private:
   osg::Matrix worldCoordMatrix;
   float heading;
};

bool useTankOrbiterView = false;

void toggleTankOrbiterView()
{
   if (! useTankOrbiterView)
      useTankOrbiterView = true;
   else
      useTankOrbiterView = false;
}

int main()
{
   osg::Node* groundNode = NULL;
   osg::Node* tankNode = NULL;
   osg::Group* root = NULL;
   osgViewer::Viewer viewer;
   osg::PositionAttitudeTransform* tankXform = NULL;

   groundNode = osgDB::readNodeFile("../NPS_Data/Models/JoeDirt/JoeDirt.flt");
   tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");

   root = new osg::Group();

   // Create green Irish sky
   osg::ClearNode* backdrop = new osg::ClearNode;
   backdrop->setClearColor(osg::Vec4(0.0f,0.8f,0.0f,1.0f));
   root->addChild(backdrop);

   tankXform = new osg::PositionAttitudeTransform();

   root->addChild(groundNode);

   root->addChild(tankXform);
   tankXform->addChild(tankNode);

   tankXform->setPosition( osg::Vec3(10,10,8) );
   tankXform->setAttitude( 
      osg::Quat(osg::DegreesToRadians(-45.0), osg::Vec3(0,0,1) ) );

   osgGA::TrackballManipulator *Tman = new osgGA::TrackballManipulator();
   viewer.setCameraManipulator(Tman);

   viewer.setSceneData( root );
   viewer.realize();

   osg::MatrixTransform* orbitTankXForm = new osg::MatrixTransform();
   orbit* tankOrbitCallback = new orbit();
   orbitTankXForm->setUpdateCallback( tankOrbitCallback );
   tankXform->addChild(orbitTankXForm);

   keyboardEventHandler* keh = new keyboardEventHandler();
   keh->addFunction('v',toggleTankOrbiterView);
   viewer.addEventHandler(keh); 

   while( !viewer.done() )
   {
 
      if (useTankOrbiterView)
      {
         Tman->setByInverseMatrix(tankOrbitCallback->getWCMatrix()
                           *osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ));
      }

      viewer.frame();
   }
}
