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

#include "avCore/Global.h"



const uint32_t cCastsShadowTraversalMask = 8;
const uint32_t cReceivesShadowTraversalMask = 16;

#include "avCore/Environment.h"

//#define SHADOW_PSSM
