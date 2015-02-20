#include "FollowNodeManip.h"

followNodeMatrixManipulator::followNodeMatrixManipulator( transformAccumulator* ta)
{
   worldCoordinatesOfNode = ta;
   theMatrix = osg::Matrixd::identity();
}

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
