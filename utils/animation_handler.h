#pragma once

class AnimationHandler : public osgGA::GUIEventHandler 
{
public: 
    typedef std::function<void()> on_effect_f;
    typedef std::function<void(bool)> on_effect2_f;

    AnimationHandler(osg::Node* model,const std::string animationName
        ,on_effect_f on_fire         = nullptr
        ,on_effect2_f on_test_effect = nullptr
        ,on_effect2_f on_lod_effect  = nullptr
        );
    ~AnimationHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
    void AnimateIt(osgAnimation::Animation::PlayMode pm, double duration);
 
protected:
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager_;
    osg::ref_ptr<osg::Node>                           model_;
    std::string                                       animationName_;
    on_effect_f                                       on_fire_;
    on_effect2_f                                      on_test_effect_;
    on_effect2_f                                      on_lod_effect_;
};