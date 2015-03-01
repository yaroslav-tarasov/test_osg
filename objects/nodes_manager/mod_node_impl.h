#pragma once 

#include "node_impl.h"

namespace nodes_management
{
struct mod_node_impl 
    : node_impl
{
    mod_node_impl( view * manager, node_impl const&  parent, model_structure::node_data const& data, uint32_t id );
    mod_node_impl( view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id );
    mod_node_impl( view * manager, binary::input_stream & stream );

    // node_impl
private:
    void post_update(double time) override;

    // node_control
private:
    void set_position(node_position const& pos) override;
    
private:
    void push_pos(node_position const& pos);

private:
    uint32_t cur_freeze() const;

private:
    struct node_pos_stamp
    {
        node_pos pos;
        uint16_t eq_with_prev;

        node_pos_stamp(){}

        node_pos_stamp(node_pos const& pos, uint16_t eq_prev = 0)
            : pos         (pos)
            , eq_with_prev(eq_prev)
        {
        }
    };

    ph_list<node_pos_stamp>::list_t pos_hist_;

    uint16_t                  freeze_  ; 
    const size_t              hist_len_;

private:
    bool delayed_send_;
};

typedef polymorph_ptr<mod_node_impl> mod_node_impl_ptr;

} //nodes_management
