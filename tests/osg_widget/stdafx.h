#pragma once
#include "targetver.h"
//////////////////////////////////
//    common ext headers
//
#include "common/boost.h"
#include "common/stl.h"

#include "common/debug.h"

/////////////////////////////////////
//        local osg graphics
//
#define BASE_SHADOW_TEXTURE_UNIT 6
#define GLSL_VERSION             130
#define ORIG_EPHEMERIS
#define PICK_NODE_MASK          0x1
#define DO_NOT_PICK_NODE_MASK   0x2


#ifndef _DEBUG
#pragma comment(lib, "osgwTools.lib")
#pragma comment(lib, "osgbDynamics.lib")
#pragma comment(lib, "osgbInteraction.lib")
#pragma comment(lib, "osgbCollision.lib")
#else 
#pragma comment(lib, "osgwToolsd.lib")
#pragma comment(lib, "osgbDynamicsd.lib")
#pragma comment(lib, "osgbInteractiond.lib")
#pragma comment(lib, "osgbCollisiond.lib")
#endif

#define TEST_SHADOWS_FROM_OSG

#include "nfi/fn_reg.h"

///////////////////////////////////////////
//    cg library block
//
#include "common/meta.h"
#include "cg_math.h"

#include "common/points.h"
#include "common/util.h"
#include "geometry/xmath.h"
//
//
////////////////////////////////////////////////////////


///////////////////////
//     Global config

#include "config/config.h"

//////////////////////
//     Boost base events

#include "common/event.h"


enum objects_t{
    NONE_TYPE    ,
    AIRCRAFT_TYPE,
    VEHICLE_TYPE
};


namespace sp = std::placeholders;

inline cg::geo_base_3 get_base()
{
    return cg::geo_base_3(cg::geo_point_3(0.0,0.0,0));
}

namespace core {
    class CoreWidget;
}

#include "common/ref_counter.h"
#include "../widget/core/core.h"

 #include "common/bullet.h"
#include "common/osg_inc.h"

#include "av/avCore/Database.h"
#include "visitors/find_node_visitor.h"
#include "osg_helpers.h"


#ifdef QT
#include "common/qt.h"
#endif

