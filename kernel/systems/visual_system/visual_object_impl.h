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

    osg::ref_ptr<osg::Node> node() const;
    void set_visible(bool visible);

private:
    osg::ref_ptr<avScene::Scene> scene_;
    osg::ref_ptr<osg::Node>      node_;
};

} // kernel
