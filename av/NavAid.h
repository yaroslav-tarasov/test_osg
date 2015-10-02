#pragma once 

#include "LightManager.h"

namespace avScene
{

class NavAidGroup: public osgSim::LightPointNode
{

public:    
    NavAidGroup ();
    NavAidGroup( const NavAidGroup & other, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY );

    // OSG stuff
    META_Node(avScene, NavAidGroup);
    
    void addLight(osgSim::LightPoint pnt, const LightManager::Light& data);


protected:
    virtual void                traverse(osg::NodeVisitor& nv);   

private:
    std::map<uint32_t,uint32_t>    lm_;
};


}