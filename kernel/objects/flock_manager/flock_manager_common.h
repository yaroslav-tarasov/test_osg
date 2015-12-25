#pragma once
#include "network/msg_base.h"


namespace flock
{

namespace manager
{

struct settings_t
{
    settings_t()
        : _childAmount       ( 250 )
		, _soarMaxTime       (  5  )
        , _minSpeed          ( 6.f )
        , _maxSpeed          (10.f )
        , _minScale          ( .7f )
        , _maxScale          ( 1.f )
        , _minAnimationSpeed ( 2.f )
        , _maxAnimationSpeed ( 4.f )
    {}

	int    _childAmount;
    float  _minSpeed;				// minimum random speed
    float  _maxSpeed;				// maximum random speed
    float  _minScale;				// minimum random size
    float  _maxScale;				// maximum random size	
    float  _minAnimationSpeed;
    float  _maxAnimationSpeed;
    int    _soarMaxTime;

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
	REFL_ENTRY(_soarMaxTime)
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



