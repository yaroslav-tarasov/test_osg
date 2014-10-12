#include <osgViewer/Viewer>
#include <osg/Node>
#include <osgGA/TrackballManipulator>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/MatrixManipulator>
#include <osg/PositionAttitudeTransform>
#include <osg/Transform>
#include <iostream>
#include <osgText/Text>
#include <osgText/Font>
#include "KeyboardHandler.h"

bool PlaceCamera = false;

void toggleView()
{
   if (! PlaceCamera)
      PlaceCamera = true;
   else
      PlaceCamera = false;
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
   textOne->setAxisAlignment(osgText::Text::XZ_PLANE);
   textOne->setColor( osg::Vec4(.5,.5,.25,1.0f) );
   textOne->setPosition( osg::Vec3(0,-5,4) );
   //textOne->setDrawMode(osgText::Text::TEXT |
   //                          osgText::Text::ALIGNMENT | 
   //                             osgText::Text::BOUNDINGBOX);
   textOne->setAlignment(osgText::Text::CENTER_TOP);
   return true;
}

class circleAimlessly : public osg::NodeCallback
{
public:
   circleAimlessly(): angle(0.0) {}

   //This block of code is an alternative method for circle method..
   //**************************************************************
   //virtual void operator () (osg::Node * node, osg::NodeVisitor* nv)
   //{
   //   osg::MatrixTransform* tx = dynamic_cast <osg::MatrixTransform*> (node);
   //   if (tx != NULL)
   //   {
   //      angle += M_PI/180.0;
   //      tx->setMatrix( osg::Matrix::translate( 100.0, 0.0, 20.0) *
   //         osg::Matrix::rotate( angle, 0, 0, 1) );
   //   }
   //   // What happens is this line is commented out???
   //   traverse(node, nv);
   //}

   virtual void operator () (osg::Node * node, osg::NodeVisitor* nv)
   {
      osg::PositionAttitudeTransform* pat = dynamic_cast <osg::PositionAttitudeTransform*> (node);
      if (pat != NULL)
      {
         angle += osg::DegreesToRadians(0.01f); 
         pat->setPosition( osg::Vec3(cosf(angle)*100.0f, sinf(angle)*100.0f, 20.0f));
         pat->setAttitude(osg::Quat(angle, osg::Vec3(0,0,1)) );
      }
      // What happens is this line is commented out??? Try and see..:-) (Hard to notice...)
      traverse(node, nv);
   }
private:
      float angle;
};

// class to allow access to matrix that represents accumlation of 
//  matrices above specified node of scene graph.
struct updateAccumulatedMatrix : public osg::NodeCallback
{
   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {
      matrix = osg::computeWorldToLocal(nv->getNodePath() );
      traverse(node,nv);
   }
   osg::Matrix matrix;
};

struct transformAccumulator
{
   transformAccumulator();
   bool attachToGroup(osg::Group* g);
   osg::Matrix getMatrix();
protected:
   osg::ref_ptr<osg::Group> parent;
   osg::Node* node;
   updateAccumulatedMatrix* mpcb;
};

osg::Matrix transformAccumulator::getMatrix()
{
   return mpcb->matrix;
}
transformAccumulator::transformAccumulator()
{
   parent = NULL;
   node = new  osg::Node;
   mpcb = new updateAccumulatedMatrix();
   node->setUpdateCallback(mpcb);
}

bool transformAccumulator::attachToGroup(osg::Group* g)
{
   bool success = false;
   if (parent != NULL)
   {
      int n = parent->getNumChildren();
      for (int i = 0; i < n; i++)
      {
         if (node == parent->getChild(i) )
         {
            parent->removeChild(i,1);
            success = true;
         }
      }
      if (! success)
      {
         return success;
      }
   }
   g->addChild(node);
   return true;
}

class followNodeMatrixManipulator : public osgGA::MatrixManipulator
{
public:
   followNodeMatrixManipulator( transformAccumulator* ta) {worldCoordinatesOfNode = ta; theMatrix = osg::Matrixd::identity();}
   bool handle (const osgGA::GUIEventAdapter&ea, osgGA::GUIActionAdapter&aa);
   void updateTheMatrix();
   virtual void setByMatrix(const osg::Matrixd& mat); 
   virtual void setByInverseMatrix(const osg::Matrixd&mat);
   virtual osg::Matrixd getInverseMatrix() const;
   virtual osg::Matrixd getMatrix() const;
protected:
   ~followNodeMatrixManipulator() {}
   transformAccumulator* worldCoordinatesOfNode;
   osg::Matrixd theMatrix;
};

void followNodeMatrixManipulator::setByMatrix(const osg::Matrixd& mat)
{
   theMatrix = mat;
}
void followNodeMatrixManipulator::setByInverseMatrix(const osg::Matrixd& mat)
{
   theMatrix = mat.inverse(mat);
}
void followNodeMatrixManipulator::updateTheMatrix()
{
   theMatrix = worldCoordinatesOfNode->getMatrix();
}
osg::Matrixd followNodeMatrixManipulator::getInverseMatrix() const
{
   osg::Matrixd m;
   m = theMatrix * osg::Matrixd::rotate(-M_PI/2.0, osg::Vec3(1,0,0) );
   return m;
}

osg::Matrixd followNodeMatrixManipulator::getMatrix() const
{
   return theMatrix;
}

bool followNodeMatrixManipulator::handle(const osgGA::GUIEventAdapter&ea, osgGA::GUIActionAdapter&aa)
{
   switch(ea.getEventType())
   {
   case (osgGA::GUIEventAdapter::FRAME):
      {
         updateTheMatrix();
         return false;
      }
   }
   return false;
}

int main( int argc, char **argv )
{
   osgViewer::Viewer viewer;
   osg::Group* rootNode = new osg::Group;

   osg::Node* terrainNode = osgDB::readNodeFile("../NPS_Data/Models/JoeDirt/JoeDirt.flt");
   if (!terrainNode)
   {
      std::cout << " no terrain! " << std::endl;
      return NULL;
   }
   rootNode->addChild(terrainNode);

   osg::Node* tankNode = osgDB::readNodeFile("../NPS_Data/Models/t72-tank/t72-tank_des.flt");
   if( ! tankNode)
   {
      std::cout << "no Tank" << std::endl;
      return NULL;
   }

   viewer.setSceneData( rootNode );
   //viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
 
   // Create and set up a transform for updating the tank's
   // position.  (For now, this will move in a circle.)
   //osg::MatrixTransform* tankPAT = new osg::MatrixTransform();
   osg::PositionAttitudeTransform* tankPAT = new osg::PositionAttitudeTransform();

   tankPAT->setUpdateCallback(new circleAimlessly);
   rootNode->addChild(tankPAT);
   tankPAT->addChild(tankNode);
   addTextLabel(tankPAT,"Follow Me!");

   // Declare and set up a transform to 'follow' the tank node.
   osg::PositionAttitudeTransform *followerPAT = new osg::PositionAttitudeTransform();
   followerPAT->setPosition( osg::Vec3(0,-22,4) );
   followerPAT->setAttitude( osg::Quat( osg::DegreesToRadians(-10.0f) , 
      osg::Vec3(1,0,0) ));

   // Add this as a child of the tank's transform
   tankPAT->addChild(followerPAT);

   // create the windows and run the threads.
   viewer.realize();

   transformAccumulator* tankWorldCoords = new transformAccumulator();
   tankWorldCoords->attachToGroup(followerPAT);
   followNodeMatrixManipulator* followTank = new followNodeMatrixManipulator(tankWorldCoords);
   //osgGA::KeySwitchMatrixManipulator *ksmm = viewer.getKeySwitchMatrixManipulator();
   //if (!ksmm)
   //   return -1;
   // add the tank follower matrix manipulator. Selecting the 'm' key 
   // with switch the viewer to this manipulator.
   //ksmm->addMatrixManipulator('m',"tankFollower",followTank);
   osgGA::TrackballManipulator *Tman = new osgGA::TrackballManipulator();
   viewer.setCameraManipulator(Tman);
   keyboardEventHandler* keh = new keyboardEventHandler();
   keh->addFunction('v',toggleView);
   viewer.addEventHandler(keh); 

   while( !viewer.done() )
   {
      if (PlaceCamera)
        viewer.setCameraManipulator(followTank);

      // fire off the cull and draw traversals of the scene.
      viewer.frame();
   }


   return 0;
}
