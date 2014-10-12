#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgSim/MultiSwitch>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgGA/TrackballManipulator>
#include <osgViewer/CompositeViewer>
#include <osgGA/StateSetManipulator>

#include "findNodeVisitor.h"
#include "FollowNodeManip.h"

bool setupScene(osg::ref_ptr<osg::Group> &rootNode,
                osg::ref_ptr<osg::Group> &ownTank  );

extern void createView (osgViewer::CompositeViewer *viewer,
                 osg::ref_ptr<osg::Group> scene,
                 osg::ref_ptr<osg::GraphicsContext> gc,
                 osgGA::TrackballManipulator* Tman,
                 int x, int y, int width, int height);

int main( int argc, char **argv )
{
   osg::ref_ptr<osg::Group> rootNode; 
   osg::ref_ptr<osg::Group> ownTank;
   osgGA::TrackballManipulator *Tman1 = new osgGA::TrackballManipulator();
   osgGA::TrackballManipulator *Tman2 = new osgGA::TrackballManipulator();
   osgGA::TrackballManipulator *Tman3 = new osgGA::TrackballManipulator();
   osgGA::TrackballManipulator *Tman4 = new osgGA::TrackballManipulator();
   osgViewer::CompositeViewer viewer;

   // build scene with terrain and two tanks
   if (!setupScene(rootNode,ownTank))
   {
      std::cout<< "problem setting up scene" << std::endl;
      return -1;
   }

   // Get a handle to our tank's turret node
   findNodeVisitor findTurret("turret");
   ownTank->accept(findTurret);
   osgSim::DOFTransform* turretXForm = dynamic_cast <osgSim::DOFTransform*> 
      (findTurret.getFirst());

   // Declare an offset for the gunner and attach it to the turret
   osg::PositionAttitudeTransform* gunnerXForm = 
      new osg::PositionAttitudeTransform();
   gunnerXForm->setPosition( osg::Vec3(0,-1.5,3.0) );
   turretXForm->addChild(gunnerXForm);

   // Declare a tranform accumulator to retreive world coords of 
   // the center gunner. Associate tranform accumulator with the offset
   // for the gunner.
   transformAccumulator* gunnerWorldCoords = new transformAccumulator();
   gunnerWorldCoords->attachToGroup(gunnerXForm);

   // Declare and set a tranform for the left gunner's view 
   // (this will be a 45 degree rotation relative to the up axis.)
   // Attach this to the gunner transform.
   osg::PositionAttitudeTransform* leftGunnerPAT = 
      new osg::PositionAttitudeTransform();
   leftGunnerPAT->setAttitude( 
      osg::Quat( osg::DegreesToRadians(45.0) , osg::Vec3(0,0,1) ));
   gunnerXForm->addChild(leftGunnerPAT);

   // Declare a transform accumulator to retrieve world coordinates 
   // for the left gunner. Associate this accumulator with the left
   // gunner's transform.
   transformAccumulator* leftGunnerWC = new transformAccumulator();
   leftGunnerWC->attachToGroup(leftGunnerPAT);

   // repeat this process for the right gunner, the offset will be 
   // a -45 degree rotation relative to the up axis.
   osg::PositionAttitudeTransform* rightGunnerPAT = 
      new osg::PositionAttitudeTransform();
   rightGunnerPAT->setAttitude(
      osg::Quat( osg::DegreesToRadians(-45.0) , osg::Vec3(0,0,1) ));
   transformAccumulator* rightGunnerWC = new transformAccumulator();
   gunnerXForm->addChild(rightGunnerPAT);
   rightGunnerWC->attachToGroup(rightGunnerPAT);

   // Declare and initialize a transform for the driver,
   // add this to the tank node.
   osg::PositionAttitudeTransform* driverOffsetPAT = 
      new osg::PositionAttitudeTransform();
   driverOffsetPAT->setPosition(osg::Vec3(0.0,2.4,1.4));
   driverOffsetPAT->setAttitude( 
      osg::Quat( osg::DegreesToRadians(0.0f), osg::Vec3(1,0,0) ) );
   ownTank->addChild(driverOffsetPAT);

   // Declare a transform accumulator to retrieve world coordinates 
   // of the driver transform.
   transformAccumulator* driverWorldCoords = new transformAccumulator();
   driverWorldCoords->attachToGroup(driverOffsetPAT);

   // Use this for a new matrix manipulator that will follow the 
   // driver transform.
   //viewer.getKeySwitchMatrixManipulator()->addMatrixManipulator
   //   ('m',"ft",new followNodeMatrixManipulator(driverWorldCoords));

   //
   // construct the composite viewer.
   osg::GraphicsContext::WindowingSystemInterface* wsi = 
            osg::GraphicsContext::getWindowingSystemInterface();
        if (!wsi) 
        {
            osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
            return 1;
        }

        unsigned int width, height;
        wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 100;
        traits->y = 100;
        traits->width = width;
        traits->height = height;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (gc.valid())
        {
            osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;

            // need to ensure that the window is cleared make sure that the complete window is set the correct colour
            // rather than just the parts of the window that are under the camera's viewports
            gc->setClearColor(osg::Vec4f(0.2f,0.2f,0.6f,1.0f));
            gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
        }
         int h2 = traits->height/2;
         int w3 = traits->width/3;
         createView (&viewer,rootNode,gc,Tman1,   0,  0, traits->width, h2);
         createView (&viewer,rootNode,gc,Tman2,   0, h2, w3, h2);
         createView (&viewer,rootNode,gc,Tman3,  w3, h2, w3, h2);
         createView (&viewer,rootNode,gc,Tman4,2*w3, h2, w3, h2);


   while( !viewer.done() )
   {
    Tman1->setByInverseMatrix(driverWorldCoords->getMatrix()
                           *osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ));
    Tman2->setByInverseMatrix(leftGunnerWC->getMatrix()
                           *osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ));
    Tman3->setByInverseMatrix(gunnerWorldCoords->getMatrix()
                           *osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ));
    Tman4->setByInverseMatrix(rightGunnerWC->getMatrix()
                           *osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ));

      // fire off the cull and draw traversals of the scene.
      viewer.frame();
   }

   return 0;
}

bool setupScene(osg::ref_ptr<osg::Group> &rootNode,
                osg::ref_ptr<osg::Group> &ownTank  )
// Setup a scene with a damaged tank near our own tank.
// Return (via reference argument) a handle to a tank we'll
//  eventually control and a handle to the root node.  
// Function returns false if it can't load
//  models or arguments are not NULL, true otherwise.
{
   if (rootNode.get() || ownTank.get())
      return false;

   rootNode = new osg::Group();
   osg::ref_ptr<osg::Node> terrainNode = 
      osgDB::readNodeFile("../NPS_Data/Models/JoeDirt/JoeDirt.flt");
   if (!terrainNode)
   {
      std::cout << " no terrain! " << std::endl;
      return false;
   }
   rootNode->addChild(terrainNode.get());

   ownTank = (osg::Group*) 
      osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   if( ! ownTank)
   {
      std::cout << "no Tank" << std::endl;
      return false;
   }
   osg::PositionAttitudeTransform* ownTankPAT = 
      new osg::PositionAttitudeTransform();
   ownTankPAT->setPosition( osg::Vec3(100,100,8) );
   rootNode->addChild(ownTankPAT);
   ownTankPAT->addChild(ownTank.get());

   osg::ref_ptr<osg::Node> damagedTank = 
      osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   if( ! damagedTank )
   {
      std::cout << "no Tank" << std::endl;
      return false;
   }
   osg::PositionAttitudeTransform* damagedTankPAT = 
      new osg::PositionAttitudeTransform();
   damagedTankPAT->setPosition( osg::Vec3(90,110,8) );
   rootNode->addChild(damagedTankPAT);
   damagedTankPAT->addChild(damagedTank.get());

   findNodeVisitor findSwitch("sw1");
   damagedTank->accept(findSwitch);
   osgSim::MultiSwitch* damagedTankSwitch = 
      dynamic_cast <osgSim::MultiSwitch*> (findSwitch.getFirst());
   if (!damagedTankSwitch)
      return -1;
   damagedTankSwitch->setSingleChildOn(0,true);

   return true;
}

