#pragma once

#include "nm/node_impl.h"
#include "vis_node_impl.h"
#include "mod_node_impl.h"

namespace nodes_management
{
    node_impl_ptr create_node(view * mana, geo_position const& pos, model_structure::node_data const& data, uint32_t id)
    {
        if (mana->system()->kind() == sys_visual)
            return make_shared<vis_node_impl>(mana, pos, data, id);
        else if (mana->system()->kind() == sys_model)
            return make_shared<mod_node_impl>(mana, pos, data, id);
        else
            return make_shared<node_impl>(mana, pos, data, id);
    }


    node_impl_ptr create_node(view * mana, node_impl const&  parent, model_structure::node_data const& data, uint32_t id)
    {
        if (mana->system()->kind() == sys_visual)
            return make_shared<vis_node_impl>(mana, parent, data, id);
        else if (mana->system()->kind() == sys_model)
            return make_shared<mod_node_impl>(mana, parent, data, id);
        else
            return make_shared<node_impl>(mana, parent, data, id);
    }


    node_impl_ptr create_node(view * mana, binary::input_stream & stream )
    {
        if (mana->system()->kind() == sys_visual)
            return make_shared<vis_node_impl>(mana, stream);
        else if (mana->system()->kind() == sys_model)
            return make_shared<mod_node_impl>(mana, stream);
        else
            return make_shared<node_impl>(mana, stream);
    }
};