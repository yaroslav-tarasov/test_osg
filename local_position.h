#pragma once

#include "position.h"

struct local_position : decart_position
{
    local_position() {}

    local_position(uint32_t relative_obj, uint32_t relative_node, cg::point_3 const& pos, cg::quaternion const& orien)
        : decart_position(pos, orien)
        , relative_obj(relative_obj)
        , relative_node(relative_node)
    {}

    uint32_t   relative_obj;
    uint32_t   relative_node;
};

