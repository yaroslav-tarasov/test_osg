#include "stdafx.h"

#include "kernel/systems/vis_system.h"

#include "visual_object_impl.h"
#include "av/Scene.h"


namespace kernel
{
    //! конструктор визуального объекта
    visual_object_impl::visual_object_impl( std::string const & res, uint32_t seed )
        : scene_( avScene::GetScene() )
    {
        node_ = scene_->load(res, seed);

        // node_->setNodeMask(0);
    }

    visual_object_impl::~visual_object_impl()
    {
        // scene_->get_objects()->remove(node_.get());
    }

    osg::ref_ptr<osg::Node> visual_object_impl::node() const
    {
        return node_;
    }

    void visual_object_impl::set_visible(bool visible)
    {
        node_->setNodeMask(visible?0xffffffff:0);
    }
}
