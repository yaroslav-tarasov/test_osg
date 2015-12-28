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
		, _spawnSphere       ( 200.f ) // 3.f
		, _minDamping        ( 1.f )
		, _maxDamping        ( 2.f )
		, _diveFrequency     ( 0.5f)
		, _soarFrequency     (  0.f )
        , _waypointDistance  ( 1.f )		
    {}

	int    _childAmount;
	int    _soarMaxTime;
    float  _minSpeed;				// minimum random speed
    float  _maxSpeed;				// maximum random speed
    float  _minScale;				// minimum random size
    float  _maxScale;				// maximum random size	
    float  _minAnimationSpeed;
    float  _maxAnimationSpeed;
	float  _spawnSphere;		    // Range around the spawner waypoints will created
	float  _minDamping;
	float  _maxDamping;
	float  _diveFrequency;
	float  _soarFrequency;         // How often soar is initiated 1 = always 0 = never
    float  _waypointDistance;      // How close this can get to waypoint before creating a new waypoint (also fixes stuck in a loop)
	std::string  _soarAnimation;
	std::string  _flapAnimation;
	std::string  _idleAnimation;
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
	REFL_ENTRY(_minSpeed)				
	REFL_ENTRY(_maxSpeed)				
	REFL_ENTRY(_minScale)				
	REFL_ENTRY(_maxScale)				
	REFL_ENTRY(_minAnimationSpeed)
	REFL_ENTRY(_maxAnimationSpeed)
	REFL_ENTRY(_spawnSphere)		    
	REFL_ENTRY(_minDamping)
	REFL_ENTRY(_maxDamping)
	REFL_ENTRY(_diveFrequency)
    REFL_ENTRY(_soarFrequency)
	REFL_ENTRY(_waypointDistance)
	REFL_ENTRY(_soarAnimation)
	REFL_ENTRY(_flapAnimation)
	REFL_ENTRY(_idleAnimation)
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



} // manager


} // namespace flock



