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
        uint            rwy_num;
        cg::point_3f    pos;
        float           angle;
        float           step;
        cg::coloraf     col;
    };

}


