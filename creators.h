#pragma once

namespace creators 
{
    typedef std::array<osg::Node*, 3> nodes_array_t;

    osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime);
    osg::Node*    createBase(const osg::Vec3& center,float radius);
    nodes_array_t createMovingModel(const osg::Vec3& center, float radius);
    nodes_array_t createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique);

}

namespace effects
{
     void insertParticle(osg::Group* root,osg::Group* rootModel, const osg::Vec3& center, float radius);
}