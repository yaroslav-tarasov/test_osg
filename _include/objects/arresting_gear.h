#pragma once


namespace arresting_gear
{
    struct rope_node_state_t
    {
        rope_node_state_t() {}
        rope_node_state_t( cg::point_3 const& vel, cg::point_3 const& coord )
            : vel(vel), coord(coord)
        {}

        cg::point_3 vel;
        cg::point_3 coord;

        REFL_INNER(rope_node_state_t)
            REFL_ENTRY(vel)
            REFL_ENTRY(coord)
        REFL_END()
    };
            
    typedef std::vector<rope_node_state_t> rope_state_t;
    typedef std::vector<rope_state_t>      ropes_state_t;

}