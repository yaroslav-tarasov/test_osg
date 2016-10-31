#include "mod_node_impl.h"
#include "nodes_manager_view.h"

namespace 
{
using namespace nodes_management;

component_t eq_components(node_pos const& curr, node_pos& next, bool revert_new_changes = false)
{
    const double max_eps = 1e-2;

    component_t components = 0;

    if (curr.loc.is_initialized() != next.loc.is_initialized())
        return ct_none;
    else if (!curr.loc || curr.loc.get() == next.loc.get()) 
        components |= ct_loc_glob;

    auto c = curr.as_doubles();
    auto n = next.as_doubles();

    for (size_t i = 0, count = node_pos::doubles_count(); i != count; ++i)
        if (!curr.loc && i < 2)
        {
            if (cg::eq(c[i], n[i]))
                components += (1 << i);
        }
        else
        {
            if (cg::eq_zero(n[i], max_eps))
                n[i] = 0; // freeze close to zero values
            
            if (cg::eq(c[i], n[i], max_eps))
            {
                components += (1 << i);

                if (revert_new_changes)
                    n[i] = c[i];
            }
        }

    return components;
}
} // 'anonymous'

namespace nodes_management
{

mod_node_impl::mod_node_impl(view * manager, binary::input_stream & stream)
    : node_impl(manager, stream)
    , freeze_       (ct_none)
    , hist_len_     (3)
    , delayed_send_ (false)
{
}

mod_node_impl::mod_node_impl(view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id)
    : node_impl(manager, pos, data, id)
    , freeze_       (ct_none)
    , hist_len_     (3)
    , delayed_send_ (false)
{
}

mod_node_impl::mod_node_impl( view * manager, node_impl const& parent, model_structure::node_data const& data, uint32_t id )
    : node_impl(manager, parent, data, id)
    , freeze_       (ct_none)
    , hist_len_     (3)
    , delayed_send_ (false)
{
}
    
void mod_node_impl::post_update(double time)
{
    node_impl::post_update(time);

    if (delayed_send_)
    {
        delayed_send_ = false;

        auto freeze = cur_freeze();
        if (freeze != freeze_)
        {
            // because of history I need to send all the components
            auto components_to_send = ct_all; // & ~(freeze_ & freeze);
            manager_->send_node_msg(
                node_id_, 
                network::wrap_msg(msg::freeze_state_msg(
                    time, 
                    components_to_send, 
                    pos_hist_.back().pos)), 
                true);

            freeze_ = freeze;
        }
        else if (freeze_ != ct_all)
            manager_->send_node_msg(
                node_id_, 
                network::wrap_msg(msg::node_pos_msg(
                    time, 
                    ct_all & (~freeze_), 
                    pos_hist_.back().pos)), 
                false);
    }
}

void mod_node_impl::set_position(node_position const& pos)
{
    //NOTE: no call to node_impl::set_position at all!
    
    // freeze logic
    push_pos(pos);    
    delayed_send_ = true;

    on_position(msg::node_pos_msg(manager_->update_time(), ct_all, pos)); // just my system
}

void mod_node_impl::push_pos(node_position const& pos)
{
    node_pos new_pos(pos);

    uint16_t eq_comp = ct_none;
    if (!pos_hist_.empty())
        eq_comp = eq_components(pos_hist_.back().pos, new_pos, true);
        
    pos_hist_.push_back(node_pos_stamp(new_pos, eq_comp));

    if (pos_hist_.size() > hist_len_)
        pos_hist_.pop_front();
}

uint32_t mod_node_impl::cur_freeze() const
{
    if (pos_hist_.size() > 1)
    {
        uint16_t freeze = ct_all;
        for (auto it = next(pos_hist_.begin()); it != pos_hist_.end(); ++it)
            freeze &= it->eq_with_prev;

        return freeze;
    }

    return ct_none;
}


} // nodes_management