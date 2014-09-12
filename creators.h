#pragma once

namespace creators 
{
        osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime);
        osg::Node* createBase(const osg::Vec3& center,float radius);
        osg::Node* createMovingModel(const osg::Vec3& center, float radius);
        osg::Node* createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique);


}

namespace effects
{
     void insertParticle(osg::Group* root,osg::Group* rootModel, const osg::Vec3& center, float radius);
}