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

node_info_ptr  node_impl::root_node() const
{
    auto n = node_;
    while(0 != n->getNumParents() && "root" != boost::to_lower_copy(n->getName()))
    {                  
        n = n->getParent(0);
    }

    return boost::make_shared<node_impl>(n.get()); // FIXME хо-хо
}

cg::geo_point_3 node_impl::get_global_pos() const
{
    //if (extrapolated_position_.is_local())
    //{
    //    transform_4 tr = get_root_transform();
    //    node_info_ptr root = root_node();
    //    return root->position().global().pos(root->position().global().orien.rotate_vector(tr.translation()));
    //}
    //else
    //    return extrapolated_position_.global().pos;
    return cg::geo_point_3();
}

cg::quaternion node_impl::get_global_orien() const
{
    //if (extrapolated_position_.is_local())
    //{
    //    transform_4 tr = get_root_transform();
    //    node_info_ptr root = root_node();
    //    return root->position().global().orien * quaternion(tr.rotation().cpr());
    //}
    //else
    //    return extrapolated_position_.global().orien;
    return cg::quaternion();
}

void node_impl::set_position(node_position const& pos)
{   
    //time_                   = m.time;
    //position_               = m.pos.get_pos(position_, m.components);
    //extrapolated_position_  = position_;
    auto mat = node_->asTransform()->asMatrixTransform()->getMatrix();
    mat.setRotate(to_osg_quat(pos.local().orien));
    node_->asTransform()->asMatrixTransform()->setMatrix(mat);
}

node_position /*const&*/ node_impl::position() /*const*/
{
    auto mat = node_->asTransform()->asMatrixTransform()->getMatrix();
    return /*extrapolated_position_*//*position_*/local_position(0,0,from_osg_vector3(mat.getTrans()),from_osg_quat(mat.getRotate()));
}

// FIXME just stub
cg::transform_4  node_impl::get_root_transform() const 
{
    return cg::transform_4();
}

}