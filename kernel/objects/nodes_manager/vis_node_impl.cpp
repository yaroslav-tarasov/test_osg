#include "stdafx.h"
#include "precompiled_objects.h"

#include "vis_node_impl.h"
#include "nodes_manager_view.h"
#include "nodes_manager_visual.h"

namespace nodes_management
{

vis_node_impl::vis_node_impl( view * manager, node_impl const&  parent, model_structure::node_data const& data, uint32_t id )
    : node_impl(manager, parent, data, id)
    , need_update_(!position_.is_local())
    , user_visible_(true)
{
    init_disp();
}

vis_node_impl::vis_node_impl( view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id )
    : node_impl(manager, pos, data, id)
    , need_update_(!position_.is_local())
    , user_visible_(true)
{
    init_disp();
}

vis_node_impl::vis_node_impl( view * manager, binary::input_stream & stream )
    : node_impl(manager, stream)
    , need_update_(!position_.is_local())
    , user_visible_(true)
{
    init_disp();
}

void vis_node_impl::on_visual_object_created()
{
    fill_victory_nodes();
    sync_position();
}

void vis_node_impl::fill_victory_nodes()
{
    visual* vis_manager = dynamic_cast<visual*>(manager_);
    victory_nodes_.clear();
        
    if (vis_manager->visual_object())
    {
        for (auto jt = data_.victory_nodes.begin(); jt != data_.victory_nodes.end(); ++jt)
        {
            if (*jt=="root")
            {
                 victory_nodes_.push_back(vis_manager->visual_object()->node().get());
                 continue;
            }

            if (auto visnode = findFirstNode(vis_manager->visual_object()->node().get(), *jt))
            {
                victory_nodes_.push_back(visnode);



                FIXME(fill_victory_nodes Need to be realized)
                //if (texture_ && visnode->as_root())
                //    visnode->as_root()->set_base_texture(*texture_);
            }
        }
        
    }
}

void vis_node_impl::extrapolated_position_reseted()
{
    visible_.reset();
    need_update_ = true;
}

void vis_node_impl::pre_update(double time)
{
    node_impl::pre_update(time);
    if (!time_)
        return;

    double dt = time - *time_;
    if(!extrapolated_position_.is_static() && !extrapolated_position_.is_local())
    {
        LOG_ODS_MSG( "vis_node_impl::pre_update(double time) [ " << name() << " ]:    dt=" << dt << "   node type: " << (extrapolated_position_.is_local()?"local":"global") << "\n" );
    }

    sync_position(dt);
}

void vis_node_impl::on_animation(msg::node_animation const& anim)
{
    LOG_ODS_MSG("vis_node_impl::on_animation: " << anim.name << "\n" );

    nm::visit_sub_tree(manager_->get_node_tree_iterator(node_id()), [this](nm::node_info_ptr n)->bool
    {
        dynamic_cast<vis_node_impl *>(n.get())->sync_position();
        return true;
    });

    FIXME(Animation Need to be realized)
    //for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    //{
    //    if (auto v_node = (*(it))->as_animgroup())
    //        v_node->play_sequence(victory::animation_player::sequence_play_info(anim.name, anim.len, float(anim.time), anim.from, anim.size));
    //}

    // FIXME  Заглушка для анимации
    FIXME(Анимация на коленке)
    osgAnimation::Animation::PlayMode pm = anim.from > 0?osgAnimation::Animation::ONCE_BACKWARDS:osgAnimation::Animation::ONCE;
    
    // victory_nodes_->
    auto root = victory_nodes_[0];
    node_     = victory_nodes_[0];

    while(0 != root->getNumParents() && "root" != boost::to_lower_copy(root->getName()))
        root = root->getParent(0);


    auto god_node =  root->getParent(0);

    auto manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( god_node->getUpdateCallback() );

    if ( manager_ )
    {   
        const osgAnimation::AnimationList& animations =
            manager_->getAnimationList();

        if(childs_callbacks_.size()==0)
        {
            for (int i =0; i < node_->asGroup()->getNumChildren();++i)
            {
                auto child_node = node_->asGroup()->getChild(i); 
                childs_callbacks_.push_back(child_node->getUpdateCallback());
            }
        }

        for ( unsigned int i=0; i<animations.size(); ++i )
        {
            const std::string& name = animations[i]->getName();

            LOG_ODS_MSG("animation name: " << name << "\n" );
            if(!manager_->isPlaying(name))
            {
                animations[i]->setPlayMode(pm);                   
                manager_->playAnimation( animations[i].get(),2,2.0 );

            }
        }

    }

}

void vis_node_impl::on_visibility(msg::visibility_msg const& m)
{
    LOG_ODS_MSG( name() << "vis_node_impl::on_visibility: " << m.visible << "\n" );
    user_visible_ = m.visible;
    need_update_ = true;
}

void vis_node_impl::on_texture(msg::node_texture_msg const& m)
{
    node_impl::on_texture(m.tex);
    
    FIXME(on_texture Need to be realized)
    //for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    //{
    //    if (auto v_node = (*(it))->as_root())
    //    {
    //        v_node->set_base_texture(*texture_);
    //    }
    //}
}

bool vis_node_impl::is_visible() const
{
    if (!user_visible_)
        return false;

    double const visibility_distance = 30000;

    if (!visible_)
    {
        if (position_.is_local())
        {
            node_info_ptr rootnode = root_node();
            visible_ = rootnode ? vis_node_impl_ptr(rootnode)->is_visible() : false;
        }
        else
        {
            geo_point_3 base = dynamic_cast<visual_system_props*>(dynamic_cast<visual*>(manager_)->vis_system())->vis_props().base_point;
            visible_ = cg::distance(base, position_.global().pos) < visibility_distance;
        }
    }

    return *visible_;
}

void vis_node_impl::set_visibility  (bool visible)
{
    user_visible_ = visible;
}

void vis_node_impl::init_disp()
{                   
    msg_disp()
        .add<msg::node_animation>(boost::bind(&vis_node_impl::on_animation , this, _1))
        .add<msg::visibility_msg>(boost::bind(&vis_node_impl::on_visibility, this, _1));
}

void vis_node_impl::sync_position(double dt)
{
    if (!need_update_)
        return;

    visual* vis_manager = dynamic_cast<visual*>(manager_);

    if (!vis_manager->visual_object())
        return ;

    bool visible = is_visible();

    for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    {
        (*(it))->setNodeMask(visible?0xffffffff:0);   // set_process_flag(visible);
    }

    if (visible)
    {
        if (position_.is_local())
        {
            cg::transform_4f tr(cg::as_translation(point_3f(extrapolated_position_.local().pos)), rotation_3f(extrapolated_position_.local().orien.rotation()));
            for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
                if ((*(it))->asTransform()/*as_transform()*/)
                if((*(it))->asTransform()->asMatrixTransform())
                    (*(it))->asTransform()->asMatrixTransform()->setMatrix(to_osg_transform(tr))/*as_transform()->set_transform(tr)*/;
        }
        else
        {
            geo_base_3 base = dynamic_cast<visual_system_props*>(dynamic_cast<visual*>(manager_)->vis_system())->vis_props().base_point;
            point_3f offset = base(extrapolated_position_.global().pos);

            cg::transform_4f tr(cg::as_translation(offset), rotation_3f(extrapolated_position_.global().orien.rotation()));
            for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
                if ((*(it))->asTransform())
                if((*(it))->asTransform()->asMatrixTransform())
                (*(it))->asTransform()->asMatrixTransform()->setMatrix(to_osg_transform(tr));

            LOG_ODS_MSG( "vis_node_impl::sync_position():   extrapolated_position_.global().pos :   x:  "  <<  extrapolated_position_.global().pos.lat << "    y: " << extrapolated_position_.global().pos.lon << "\n" );
            LOG_ODS_MSG( "vis_node_impl::sync_position():   extrapolated_position_.global().pos :   dx:  "
                <<  extrapolated_position_.global().pos.lat   - prev_extrapolated_position_.global().pos.lat
                << "    dy: "  << extrapolated_position_.global().pos.lon - prev_extrapolated_position_.global().pos.lon
                << "    dvx: " << (extrapolated_position_.global().pos.lat   - prev_extrapolated_position_.global().pos.lat) / (dt?dt:1.0) / 0.00001
                << "    dvy: " << (extrapolated_position_.global().pos.lon - prev_extrapolated_position_.global().pos.lon) / (dt?dt:1.0) / 0.00001
                << "\n" );
            
            prev_extrapolated_position_  = extrapolated_position_;
        }
    }

    need_update_ = false;
}

vis_node_control::vis_nodes_t const& vis_node_impl::vis_nodes() const
{
    return victory_nodes_;
}
             
} // nodes_management
