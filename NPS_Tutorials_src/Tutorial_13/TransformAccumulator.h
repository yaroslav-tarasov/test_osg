#ifndef TRANSFORM_ACCUMULATOR_H
#define TRANSFORM_ACCUMULATOR_H

#include <osg/Node>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <osg/Matrix>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>

// class to allow access to matrix that represents accumlation of 
//  matrices above specified node of scene graph.
struct updateAccumlatedMatrix : public osg::NodeCallback
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
   updateAccumlatedMatrix* mpcb;
};
#endif
