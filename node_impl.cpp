#include "stdafx.h"

#include "node_impl.h"

namespace nodes_management
{
    
void node_impl::pre_update(double time)
{
    if (!time_)
        return;

    double dt = time - *time_;

    if (dt <= 0)
        return;

    if (!position_.is_static())
    {
        if (position_.is_local())
        {                                                                                                                                       
            extrapolated_position_.local().pos   = position_.local().pos + position_.local().dpos * dt;
            extrapolated_position_.local().orien = cg::quaternion(cg::rot_axis(position_.local().omega * dt)) * position_.local().orien;
            extrapolated_position_reseted();
        }
        else
        {
            extrapolated_position_.global().pos   = position_.global().pos(position_.global().dpos * dt);
            extrapolated_position_.global().orien = cg::quaternion(cg::rot_axis(position_.global().omega * dt)) * position_.global().orien;
            extrapolated_position_reseted();
        }
    }
}

void node_impl::post_update(double /*time*/)
{
}

void node_impl::extrapolated_position_reseted() 
{
}

void node_impl::play_animation  (std::string const& seq, double len, double from, double size) 
{
    // FIXME  Заглушка для анимации
    osgAnimation::Animation::PlayMode pm = from > 0?osgAnimation::Animation::ONCE_BACKWARDS:osgAnimation::Animation::ONCE;
    
    auto god_node = node_impl_ptr(root_node())->as_osg_node()->getParent(0);
                                           
    auto manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( god_node->getUpdateCallback() );
    
    if ( manager_ )
    {   

        const osgAnimation::AnimationList& animations =
            manager_->getAnimationList();
        
        //std::vector<std::string>  childs_names;
        
        if(childs_callbacks_.size()==0)
            for (int i =0; i < node_->asGroup()->getNumChildren();++i)
            {
               auto child_node = node_->asGroup()->getChild(i); 
               childs_callbacks_.push_back(child_node->getUpdateCallback());
               // child_node->setUpdateCallback(nullptr);
            }
        
        //osgAnimation::ChannelList& channels = animations[0]->getChannels();
        //std::list<std::string>  chan_names;  
        //std::for_each(channels.begin(),channels.end(),[&chan_names,&childs_names](osg::ref_ptr<osgAnimation::Channel> chan)
        //                                              {
        //                                                  chan_names.push_back(chan->getTargetName());
        //                                                  if(std::find(childs_names.begin(), childs_names.end(), chan->getTargetName())!=childs_names.end())
        //                                                  {
        //                                                      chan->setTargetName("hhhhhh");
        //                                                  }
        //                                              }
        //);

        for ( unsigned int i=0; i<animations.size(); ++i )
        {
            const std::string& name = animations[i]->getName();
            // if ( name==animationName_ )
            {
                animations[i]->setPlayMode(pm);                   
                manager_->playAnimation( animations[i].get(),2,2.0 );

            }
        }

        //for( auto it = chan_names.begin();it!=chan_names.end();++it)
        //    channels[std::distance(chan_names.begin(),it)]->setTargetName(*it);

        //for (int i =0; i < node_->asGroup()->getNumChildren();++i)
        //{
        //    auto child_node = node_->asGroup()->getChild(i); 
        //    child_node->setUpdateCallback(childs_callbacks_[i]);
        //}
    }
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
    name_ = boost::to_lower_copy(node_->getName()); 
    return name_;
}

uint32_t node_impl::node_id() const
{
    return node_id_;
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
    //extrapolated_position_  = position_;   ]
    if(node_->asTransform()->asMatrixTransform())
    {
        auto mat = node_->asTransform()->asMatrixTransform()->getMatrix();
        mat.setRotate(to_osg_quat(pos.local().orien));
        node_->asTransform()->asMatrixTransform()->setMatrix(mat);
    }
    else if(node_->asTransform()->asPositionAttitudeTransform())
    {
        auto pat = node_->asTransform()->asPositionAttitudeTransform();
        node_->asTransform()->asPositionAttitudeTransform()->setAttitude(to_osg_quat(pos.local().orien));
    }
}

node_position /*const&*/ node_impl::position() /*const*/
{
    // auto mat = node_->get asTransform()->asMatrixTransform()->getMatrix();
    auto pat = node_->asTransform()->asPositionAttitudeTransform();
    auto mat = node_->asTransform()->asMatrixTransform();

    osg::Vec3 trans = pat?pat->getPosition():mat->getMatrix().getTrans();  
    osg::Quat rot = pat?pat->getAttitude():mat->getMatrix().getRotate();  

    return /*extrapolated_position_*//*position_*/local_position(0,0,from_osg_vector3(trans),from_osg_quat(rot));
}

// FIXME just stub
cg::transform_4  node_impl::get_root_transform() const 
{
    return cg::transform_4();
}

}