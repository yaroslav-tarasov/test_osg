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
    visual_object_impl( std::string const & res, uint32_t seed, bool async, on_object_loaded_f f);
    visual_object_impl( nm::node_control_ptr parent, std::string const & res, uint32_t seed, bool async, on_object_loaded_f f);
   
   ~visual_object_impl();

    osg::ref_ptr<osg::Node> node() const override;
    osg::ref_ptr<osg::Node> root() const override;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> animation_manager() const override;
    osg::Node* get_node(const std::string& name) const override;

    void set_visible(bool visible)       override;
    bool get_visible()                   override;

    void set_object_loaded()             override;

#ifdef ASYNC_OBJECT_LOADING
private:
    void object_loaded( uint32_t seed );
#endif
    uint32_t                            seed_;
    on_object_loaded_f                    ol_;
    bool                               async_;
private:

    nm::node_control_ptr                                    parent_;

    struct private_t;
    boost::shared_ptr<private_t> p_;
};

} // kernel
