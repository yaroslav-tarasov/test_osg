#pragma once


namespace arresting_gear
{


struct settings_t
{
    typedef std::pair<cg::point_3f,cg::point_3f> rope_t;

    settings_t()
        : model("arresting_gear")
    {}

    vector<rope_t> ropes;
    string   model;
};


REFL_STRUCT(settings_t)
    REFL_ENTRY(model)
    REFL_ENTRY(ropes)          
REFL_END()

struct state_t
{
    state_t()
    {}

    state_t(cg::geo_point_3 const& pos, cg::quaternion const& orien)
        : pos(pos)
        , orien(orien)
    {}

    cg::geo_point_3 pos;
    cg::quaternion  orien;
};

REFL_STRUCT(state_t)
    REFL_ENTRY(pos)
    REFL_ENTRY(orien)
REFL_END()


} // namespace arresting_gear


#include "arresting_gear/arresting_gear_msg.h"
