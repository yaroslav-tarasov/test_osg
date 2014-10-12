#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/PolygonOffset>
#include <osg/Camera>

#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <iostream>
#include "TransformAccumulator.h"


class circleAimlessly : public osg::NodeCallback 
{
public:
   circleAimlessly(): _angle(0.0) {}
   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {
      osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
      if( tx != NULL )
      {
         _angle += M_PI/18000.0;
         tx->setMatrix( osg::Matrix::translate( 100.0, 0.0, 20.0) * 
            osg::Matrix::rotate( _angle, 0, 0, 1 ) );
      }
      traverse(node, nv);
   }
private:
   float _angle;
};
bool setupScene(osg::ref_ptr<osg::Group> &rootNode,
                osg::ref_ptr<osg::Group> &ownTank  )

// Setup a scene, return (via reference arguments) handles
// to the root node and the tank model.  
// Attach an update callback to the tank transform so the tank
// circles above our terrain model.
// Function returns false if it can't load
//  models or arguments are not NULL, true otherwise.
{

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

   osg::MatrixTransform * tankXForm = 
      new osg::MatrixTransform();
   tankXForm->setUpdateCallback( new circleAimlessly() );
   rootNode->addChild(tankXForm);
   tankXForm->addChild(ownTank.get());

   return true;
}

void createView (osgViewer::CompositeViewer *viewer,
                 osg::ref_ptr<osg::Group> scene,
                 osg::ref_ptr<osg::GraphicsContext> gc,
                 osgGA::TrackballManipulator* Tman,
                 int x, int y, int width, int height)
{
            double left,right,top,bottom,near,far, aspectratio;
            double frusht, fruswid, fudge;
            bool gotfrustum;
            osgViewer::View* view = new osgViewer::View;
            viewer -> addView(view);
            view->setCameraManipulator(Tman);

            view->setSceneData(scene.get());
            view->getCamera()->setViewport(new osg::Viewport(x,y, width,height));
            view->getCamera()-> getProjectionMatrixAsFrustum(left,right,
                                          bottom,top,
                                          near,far);
            if (gotfrustum) 
              {
              aspectratio = (double) width/ (double) height; 
              frusht = top - bottom;
              fruswid = right - left;
              fudge = frusht*aspectratio/fruswid;
              right = right*fudge;
              left = left*fudge;
              view->getCamera()-> setProjectionMatrixAsFrustum(left,right,
                                          bottom,top,
                                          near,far);
              }
            view->getCamera()->setGraphicsContext(gc.get());

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

            view->addEventHandler( statesetManipulator.get() );
}

int main( int argc, char **argv )
{
   // pointers to the root node and the tank model node 
   osg::ref_ptr<osg::Group> rootNode; 
   osg::ref_ptr<osg::Group> ownTank;
   osgGA::TrackballManipulator *Tman1 = new osgGA::TrackballManipulator();
   osgGA::TrackballManipulator *Tman2 = new osgGA::TrackballManipulator();

   ///osg::ref_ptr<osg::Node> scene;
   // build scene with terrain and two tanks
   if (!setupScene(rootNode, ownTank))
   {
      std::cout<< "problem setting up scene" << std::endl;
      return -1;
   }
   // Declare a transform for a position above and behind the tank
   // aimed down slightly. Add this transform to the tank node. 
   osg::PositionAttitudeTransform * followerOffset =
      new osg::PositionAttitudeTransform();
   followerOffset->setPosition( osg::Vec3(0.0,-25.0,10) );
   followerOffset->setAttitude(
      osg::Quat( osg::DegreesToRadians(-15.0), osg::Vec3(1,0,0) ) );

   ownTank.get()->addChild(followerOffset);
   // Declare a transform accumulator to be used to position a
   // camera. Attach transform accumulator to the follower transform. 
   transformAccumulator* tankFollowerWorldCoords = 
      new transformAccumulator();
   tankFollowerWorldCoords->attachToGroup(followerOffset);

   // construct the viewer.
   osgViewer::CompositeViewer viewer;
        osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
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
        // view one
        {
          
         createView (&viewer,rootNode,gc,Tman1,0, 0, traits->width/2, traits->height);
        }

        // view two
        {
         createView (&viewer,rootNode,gc,Tman2,traits->width/2, 0, traits->width/2, traits->height);
        }

   viewer.setThreadingModel(osgViewer::CompositeViewer::SingleThreaded); 
   while( !viewer.done() )
   {
    Tman2->setByInverseMatrix(tankFollowerWorldCoords->getMatrix()
                           *osg::Matrix::rotate( -M_PI/2.0, 1, 0, 0 ));

      viewer.frame();
   }

}
