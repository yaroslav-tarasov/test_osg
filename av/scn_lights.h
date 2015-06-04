#pragma once

namespace scn
{
    struct light_data
    {
        cg::point_3f    pos;
        cg::coloraf     col;
        float           size;
        float           vis_dist;
    };

    struct line_data
    {
        std::string     name;
        uint16_t        rwy_num;  
        cg::point_3f    pos;
        float           angle;
        float           step;
        cg::coloraf     col;
    };

    struct lights_group
    {
        std::string    name;
        uint16_t       rwy_num;  
    };

}


