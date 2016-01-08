#pragma once
#include "network/msg_base.h"


namespace aerostat
{


struct settings_t
{
    settings_t()
        : model("aerostat")
    {}

    string   model;
};



namespace msg
{

enum msg_type
{
    mt_settings
};

struct settings_msg
    : network::msg_id<mt_settings>
{
    settings_t settings;

    settings_msg(settings_t const& settings)
        : settings(settings)
    {
    }
    
    settings_msg()
    {
    }
};

REFL_STRUCT(settings_msg)
    REFL_ENTRY(settings)
REFL_END()
} // messages 



REFL_STRUCT(settings_t)
    REFL_ENTRY(model)
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


} // namespace aerostat



