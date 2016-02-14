#pragma once 

#include "human_common.h"

namespace human
{
//! настройки подвижного объекта
struct settings_t
{
    settings_t()
        : model("human")
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
	{}

	state_t(cg::geo_point_3 const& pos, cg::quaternion const& orien, double speed)
		: pos  (pos)
		, orien(orien)
		, speed(speed)
	{}

	cg::geo_point_3 pos;
	cg::quaternion  orien;
	double          speed; 
};

REFL_STRUCT(state_t)
	REFL_ENTRY(pos)
	REFL_ENTRY(orien)
    REFL_ENTRY(speed)
REFL_END()

} // namespace human

