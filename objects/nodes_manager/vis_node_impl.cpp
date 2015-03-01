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
            if (auto visnode = victory::find_name(vis_manager->visual_object()->node().get(), *jt))
            {
                victory_nodes_.push_back(visnode);

                if (texture_ && visnode->as_root())
                    visnode->as_root()->set_base_texture(*texture_);
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
    sync_position();
}

void vis_node_impl::on_animation(msg::node_animation const& anim)
{
    nm::visit_sub_tree(manager_->get_node_tree_iterator(node_id()), [this](nm::node_info_ptr n)->bool
    {
        dynamic_cast<vis_node_impl *>(n.get())->sync_position();
        return true;
    });

    for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    {
        if (auto v_node = (*(it))->as_animgroup())
            v_node->play_sequence(victory::animation_player::sequence_play_info(anim.name, anim.len, float(anim.time), anim.from, anim.size));
    }
}

void vis_node_impl::on_visibility(msg::visibility_msg const& m)
{
    user_visible_ = m.visible;
    need_update_ = true;
}

void vis_node_impl::on_texture(msg::node_texture_msg const& m)
{
    node_impl::on_texture(m.tex);

    for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
    {
        if (auto v_node = (*(it))->as_root())
        {
            v_node->set_base_texture(*texture_);
        }
    }
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

void vis_node_impl::init_disp()
{                   
    msg_disp()
        .add<msg::node_animation>(boost::bind(&vis_node_impl::on_animation , this, _1))
        .add<msg::visibility_msg>(boost::bind(&vis_node_impl::on_visibility, this, _1));
}

void vis_node_impl::sync_position()
{
    if (!need_update_)
        return;

    visual* vis_manager = dynamic_cast<visual*>(manager_);

    if (!vis_manager->visual_object())
        return ;

    bool visible = is_visible();


    for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
        (*(it))->set_process_flag(visible);

    if (visible)
    {
        if (position_.is_local())
        {
            cg::transform_4f tr(cg::as_translation(point_3f(extrapolated_position_.local().pos)), rotation_3f(extrapolated_position_.local().orien.rotation()));
            for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
                if ((*(it))->as_transform())
                    (*(it))->as_transform()->set_transform(tr);
        }
        else
        {
            geo_base_3 base = dynamic_cast<visual_system_props*>(dynamic_cast<visual*>(manager_)->vis_system())->vis_props().base_point;
            point_3f offset = base(extrapolated_position_.global().pos);

            cg::transform_4f tr(cg::as_translation(offset), rotation_3f(extrapolated_position_.global().orien.rotation()));
            for (auto it = victory_nodes_.begin(); it != victory_nodes_.end(); ++it)
                (*(it))->as_transform()->set_transform(tr);
        }
    }

    need_update_ = false;
}

vis_node_control::vis_nodes_t const& vis_node_impl::vis_nodes() const
{
    return victory_nodes_;
}
             
} // nodes_management
