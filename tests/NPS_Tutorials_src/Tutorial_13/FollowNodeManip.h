#ifndef FOLLOW_NODE_MATRIX_MANIP_H
#define FOLLOW_NODE_MATRIX_MANIP_H

#include <osgGA/MatrixManipulator>
#include <osgViewer/Viewer>     // for M_PI

#include "TransformAccumulator.h"
#include "FollowNodeManip.h"

class followNodeMatrixManipulator : public osgGA::MatrixManipulator
{
public:
   followNodeMatrixManipulator( transformAccumulator* ta);
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
#endif
