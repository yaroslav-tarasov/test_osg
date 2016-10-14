
#include "InstancesNum.h"
#include <osg/GLExtensions>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

InstancesNum::InstancesNum()
    : _num(0)
    , _instGeode(nullptr)
{
}

InstancesNum::InstancesNum(osg::Geode* ig, uint32_t num)
    : _num(num)
    , _instGeode(ig)
{
}

InstancesNum::~InstancesNum()
{
}

void InstancesNum::apply(State& state) const
{
   
    if(_instGeode==nullptr)
        return;

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    for(unsigned i=0;i<_instGeode->getNumDrawables(); ++i)
    { 
        _instGeode->getDrawable(i)->dirtyBound();

        // if (instNum_!=instCounter)
        {
            auto geometry = _instGeode->getDrawable(i)->asGeometry();
            // first turn on hardware instancing for every primitive set
            for (unsigned int j = 0; j < geometry->getNumPrimitiveSets(); ++j)
            {
                geometry->getPrimitiveSet(j)->setNumInstances(_num);
            }
        }

    }
}


