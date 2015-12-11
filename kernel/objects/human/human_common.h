#pragma once 

#include "human_common.h"

namespace human
{
//! настройки подвижного объекта
struct settings_t
{
    settings_t()
        : model("remy")
        , debug_draw(false)
    {
    }

    std::string model;
    std::string route;
    std::string aerotow;
    bool debug_draw;
};

REFL_STRUCT(settings_t)
    REFL_ENTRY(model)
    REFL_ENTRY(route)
    REFL_ENTRY(aerotow)
    REFL_ENTRY(debug_draw)
REFL_END()

//! состояние подвижного объекта
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

} // namespace human

