#pragma once

namespace avScene
{
    class Scene;
}


namespace kernel
{

struct visual_object_impl
    : visual_object
{
    visual_object_impl( std::string const & res, uint32_t seed, bool async );
    visual_object_impl( nm::node_control_ptr parent, std::string const & res, uint32_t seed, bool async );
   
   ~visual_object_impl();

    osg::ref_ptr<osg::Node> node() const override;
    osg::ref_ptr<osg::Node> root() const override;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> animation_manager() const override;

    void set_visible(bool visible)       override;

#ifdef ASYNC_OBJECT_LOADING
private:
    void object_loaded( uint32_t seed );
    uint32_t                          seed_;
#endif
    bool                              loaded_;

private:
    osg::observer_ptr<avScene::Scene> scene_;
    osg::ref_ptr<osg::Node>           node_;
    osg::ref_ptr<osg::Node>           root_;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> anim_manager_;
    nm::node_control_ptr              parent_;

};

} // kernel
