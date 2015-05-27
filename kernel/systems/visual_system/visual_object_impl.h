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
    visual_object_impl( std::string const & res, uint32_t seed );
    visual_object_impl(  nm::node_control_ptr parent, std::string const & res, uint32_t seed );
   
   ~visual_object_impl();

    osg::ref_ptr<osg::Node> node() const override;
    osg::ref_ptr<osg::Node> root() const override;
    void set_visible(bool visible);

private:
    osg::observer_ptr<avScene::Scene> scene_;
    osg::ref_ptr<osg::Node>           node_;
    osg::ref_ptr<osg::Node>           root_;

    nm::node_control_ptr         parent_;
};

} // kernel
