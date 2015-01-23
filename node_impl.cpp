#include "stdafx.h"

#include "node_impl.h"

namespace nodes_management
{

void node_impl::play_animation  (std::string const& seq, double len, double from, double size) 
{

}

void node_impl::set_texture     (std::string const& texture) 
{

}

void node_impl::set_visibility  (bool visible) 
{
      node_->setNodeMask(visible?0xfffffffff:0);
}

std::string const&  node_impl::name() const
{
     return node_->getName();
}

cg::sphere_3   node_impl::get_bound()
{
     const osg::BoundingSphere& bs = node_->getBound();
     return cg::sphere_3(cg::sphere_3::point_t(bs.center().x(),bs.center().y(),bs.center().z()),bs.radius());
}

}