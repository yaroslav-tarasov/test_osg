#pragma once
#include "network/msg_base.h"


namespace flock
{

namespace manager
{

struct settings_t
{
    settings_t()
        : _childAmount(250)
    {}

	int _childAmount;

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
    REFL_ENTRY(_childAmount)
REFL_END()

struct state_t
{
    state_t()
        : course(0)
        , speed(0)
    {}

    state_t(cg::geo_point_2 const& pos, double course, double speed)
        : pos(pos), course(course), speed(speed)
    {}

    cg::geo_point_2 pos;
    double course;
    double speed;
};

REFL_STRUCT(state_t)
    REFL_ENTRY(pos)
    REFL_ENTRY(course)
    REFL_ENTRY(speed)
REFL_END()

} // manager


} // namespace flock



