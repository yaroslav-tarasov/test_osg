#include "stdafx.h"
#include "precompiled_objects.h"

#include "node_impl.h"
#include "nodes_manager/nodes_manager_common.h"
#include "nodes_manager/nodes_manager_view.h"

#ifdef OSG_NODE_IMPL
namespace nodes_management
{

    node_impl::node_impl( view * manager, node_impl const& parent, model_structure::node_data const& data, uint32_t id )
        : manager_(manager)
        , data_   (data)
        , node_id_(id)
    {
        position_.set_local(local_position(manager->object_id(), parent.node_id(), data.pos, data.orien));
        extrapolated_position_ = position_;

        init_disp();
    }

    node_impl::node_impl( view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id )
        : manager_(manager)
        , data_   (data)
        , node_id_(id)
    {
        position_.set_global(pos);
        extrapolated_position_ = position_;

        init_disp();
    }

    node_impl::node_impl( view * manager, binary::input_stream & stream )
        : manager_(manager)
    {
        using namespace binary;

        read(stream, node_id_); 
        read(stream, data_); 
        read(stream, position_); 
        read(stream, texture_);

        extrapolated_position_ = position_;

        init_disp();
    }

void node_impl::save( binary::output_stream& stream ) const
{
    using namespace binary;

    write(stream, node_id_); 
    write(stream, data_); 
    write(stream, position_); 
    write(stream, texture_);
}    

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
            extrapolated_position_.local().pos   = position_.local().pos + position_.local().ddpos_t(dt); 
            extrapolated_position_.local().orien = cg::quaternion(cg::rot_axis(position_.local().omega * dt)) * position_.local().orien;
            extrapolated_position_reseted();
        }
        else
        {
            extrapolated_position_.global().pos   = position_.global().pos(position_.global().ddpos_t(dt));
            extrapolated_position_.global().orien = cg::quaternion(cg::rot_axis(position_.global().omega * dt)) * position_.global().orien;
            extrapolated_position_reseted();
        }
    }
}

void node_impl::post_update(double /*time*/)
{
}

void node_impl::on_object_created(object_info_ptr object)
{
    if ((!rel_node_ || !*rel_node_) && position_.is_local())
    {               
        if (position_.local().relative_obj == object->object_id())
        {
            manager_ptr rel_object = object;
            rel_node_ = rel_object->get_node(position_.local().relative_node);
        }
    }
}

void node_impl::on_object_destroying(object_info_ptr object)
{           
    if (rel_node_ && *rel_node_)
    {
        if ((*rel_node_)->object_id() == object->object_id())
            rel_node_.reset();
    }
}

model_structure::node_data const& node_impl::data() const 
{ 
    return data_; 
}

void node_impl::extrapolated_position_reseted() 
{
}

network::msg_dispatcher<>& node_impl::msg_disp()
{
    return msg_disp_;
}

void node_impl::init_disp()
{   
    msg_disp()
        .add<msg::freeze_state_msg  >(boost::bind(&node_impl::on_position , this, _1))
        .add<msg::node_pos_msg      >(boost::bind(&node_impl::on_position , this, _1))
        .add<msg::node_texture_msg  >(boost::bind(&node_impl::on_texture  , this, _1))
        .track<msg::node_animation  >()
        .track<msg::visibility_msg  >();
}

void node_impl::play_animation  (std::string const& seq, double len, double from, double size, double cross_fade) 
{
    // FIXME  Заглушка для анимации
    FIXME(Анимация на коленке)
    osgAnimation::Animation::PlayMode pm = from > 0?osgAnimation::Animation::ONCE_BACKWARDS:osgAnimation::Animation::ONCE;
    
    pm = from < 0?osgAnimation::Animation::LOOP:pm;
    
    auto god_node = node_impl_ptr(root_node())->as_osg_node()->getParent(0);
                                           
    auto manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( god_node->getUpdateCallback() );
   
    if ( manager_ )
    {   

        const osgAnimation::AnimationList& animations =
            manager_->getAnimationList();
        
        if(childs_callbacks_.size()==0)
            for (int i =0; i < node_->asGroup()->getNumChildren();++i)
            {
               auto child_node = node_->asGroup()->getChild(i); 
               childs_callbacks_.push_back(child_node->getUpdateCallback());

            }
        
        for ( unsigned int i=0; i<animations.size(); ++i )
        {
            const std::string& name = animations[i]->getName();
            // if ( name==animationName_ )
            {
                animations[i]->setPlayMode(pm);                   
                manager_->playAnimation( animations[i].get(),2,2.0 );
            }
        }

    }
}

void node_impl::set_texture     (std::string const& texture) 
{

}

void node_impl::set_visibility  (bool visible) 
{
      node_->setNodeMask(visible?/*0xfffffffff*/0x00010000:0);
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

uint32_t node_impl::object_id() const
{
    osg::Node* n = node_.get();
    uint32_t id=0;
    bool got_phys_node=false;
    while(0 != n->getNumParents() && (got_phys_node = "phys_ctrl" != boost::to_lower_copy(n->getName())))
    {                  
        n = n->getParent(0);
    }

    if(!got_phys_node)
    {
        n->getUserValue("id",id);
    }

    return id;
}


cg::sphere_3   node_impl::get_bound() const
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

    return boost::make_shared<node_impl>(n.get(),manager_); // FIXME хо-хо
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
    FIXME(get_global_pos())
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
    FIXME(get_global_orien())
    return cg::quaternion();
}

void node_impl::set_position(node_position const& pos)
{   
    //time_                   = m.time;
    //position_               = m.pos.get_pos(position_, m.components);
    //extrapolated_position_  = position_; 

    FIXME(Больное место локальные/глобальные координаты)
    if(pos.is_local())
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

model_structure::collision_volume const* node_impl::get_collision() const
{
    //if (!collision_ && manager_->get_collision_structure())
    //{
    //    auto const& volumes = manager_->get_collision_structure()->collision_volumes;
    //    auto it = volumes.find(data_.name);
    //    collision_.reset(it != volumes.end() ? &*(it->second) : NULL);
    //}

    //return collision_ ? *collision_ : NULL;
    return nullptr;
}

node_position const& node_impl::position() const
{
    auto pat = node_->asTransform()->asPositionAttitudeTransform();
    auto mat = node_->asTransform()->asMatrixTransform();

    osg::Vec3 trans = pat?pat->getPosition():mat->getMatrix().getTrans();  
    osg::Quat rot = pat?pat->getAttitude():mat->getMatrix().getRotate();  
    
    extrapolated_position_ = local_position(0,0,from_osg_vector3(trans),from_osg_quat(rot));
    FIXME(Больное место локальные/глобальные координаты)
    return extrapolated_position_;
}

// FIXME just stub
cg::transform_4  node_impl::get_root_transform() const 
{
    FIXME(Пустой трансформ хм)
    return cg::transform_4();
}  

node_info_ptr node_impl::rel_node() const
{
    Assert(position_.is_local());

    FIXME(Загадошный функционал)
    //if (rel_node_)
    //    return *rel_node_;

    //manager * rel_object ;
    //if (position_.local().relative_obj == manager_->object_id())
    //    rel_object = manager_;
    //else
    //    rel_object = manager_ptr(manager_->collection()->get_object(position_.local().relative_obj)).get();

    //rel_node_ = rel_object ? rel_object->get_node(position_.local().relative_node) : node_info_ptr();
    
    rel_node_ = boost::make_shared<node_impl>(node_->getParent(0), manager_);

    return *rel_node_;
}

transform_4 node_impl::transform  () const
{
    Assert(extrapolated_position_.is_local());
    return transform_4(cg::as_translation(extrapolated_position_.local().pos), cg::rotation_3(extrapolated_position_.local().orien.cpr()));
}

void node_impl::on_msg(binary::bytes_cref data)
{               
    msg_disp().dispatch_bytes(data);
}

void node_impl::on_position(msg::node_pos_descr const& m)
{
    time_                   = m.time;
    position_               = m.pos.get_pos(position_, m.components);
    extrapolated_position_  = position_;

    if (position_.is_local())
    {
        if (rel_node_ && *rel_node_)
        {
            if ((*rel_node_)->object_id() != position_.local().relative_obj || (*rel_node_)->node_id() != position_.local().relative_node)
                rel_node_.reset();
        }
    }
    else
        rel_node_.reset();

    extrapolated_position_reseted();
}

void node_impl::on_texture(msg::node_texture_msg const& m)
{
    texture_ = m.tex;
}

}

#endif
