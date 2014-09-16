#pragma once

namespace creators 
{
    typedef std::array<osg::Node*, 6> nodes_array_t;

    osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime);
    osg::Node*    createBase(const osg::Vec3& center,float radius);
    nodes_array_t createMovingModel(const osg::Vec3& center, float radius);
    nodes_array_t createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique);

}

namespace effects
{
     void insertParticle(osg::Group* root,osg::Node* rootModel, const osg::Vec3& center, float radius);
     osg::Node* createLightSource( unsigned int num,
         const osg::Vec3& trans,
         const osg::Vec4& color );
}