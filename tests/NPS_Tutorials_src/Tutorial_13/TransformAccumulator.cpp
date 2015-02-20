#include "TransformAccumulator.h"

osg::Matrix transformAccumulator::getMatrix()
{
   return mpcb->matrix;
}
transformAccumulator::transformAccumulator()
{
   parent = NULL;
   node = new osg::Node;
   mpcb = new updateAccumlatedMatrix();
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
