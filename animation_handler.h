#pragma once

class AnimationHandler : public osgGA::GUIEventHandler 
{
public: 
    typedef std::function<void()> on_fire_f;


    AnimationHandler(osg::Node* model,const std::string animationName,on_fire_f on_fire);
    ~AnimationHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
    void AnimateIt(osgAnimation::Animation::PlayMode pm);
 
protected:
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager_;
    osg::ref_ptr<osg::Node>                           model_;
    std::string                                       animationName_;
    on_fire_f                                         on_fire_;
};