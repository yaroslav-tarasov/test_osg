//#undef precompile_header 
// #include "stdafx.h"

namespace avSky
{
	enum cloud_type
	{
		none,
		cloudy,
		cirrus,
		overcast,
		clouds_types_num
	} ;
}

namespace avSky
{

    typedef std::function<void(float&)> on_visible_range_change_f;
}


enum render_order_t {
    RENDER_BIN_SMOKE                    =  9,
    RENDER_BIN_LIGHTS                   =  8,
    RENDER_BIN_SCENE                    =  0,
    // rendered first - sky-dome, stars, clouds, etc...
    RENDER_BIN_SKYDOME                  = -5, // global sky dome
    RENDER_BIN_STARS                    = -4, // stars
    RENDER_BIN_SUN_MOON                 = -3, // sun. moon and other planet
    RENDER_BIN_CLOUDS                   = -2, // sky clouds
    RENDER_BIN_SKYFOG                   = -1, // global sky fog layer

};

enum masks_t{
    NODE_STARFIELD_MASK                 = 0x2
};

const uint32_t cCastsShadowTraversalMask = 8;
const uint32_t cReceivesShadowTraversalMask = 16;

#include "av/Environment.h"
