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
   ~visual_object_impl();

    osg::ref_ptr<osg::Node> node() const override;
    osg::ref_ptr<osg::Node> root() const override;
    osg::ref_ptr<osg::Node> pat() const override;
    void set_visible(bool visible);

private:
    osg::ref_ptr<avScene::Scene> scene_;
    osg::ref_ptr<osg::Node>      node_;
    osg::ref_ptr<osg::Node>      root_;
    osg::ref_ptr<osg::Node>      pat_;
};

} // kernel
