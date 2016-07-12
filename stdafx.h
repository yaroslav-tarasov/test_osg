// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef  precompile_header

// time to undef now
#undef SYSTEMS_API
#define SYSTEMS_API

#include "targetver.h"

//////////////////////////////////
//  Common includes
//

#include "common/boost.h"
#include "common/stl.h"

#include "common/osg_inc.h"
#include "common/bullet.h"
#include "common/debug.h"

#ifdef QT
#include "common/qt.h"
#endif

#include "common/misc.h"



//////////////////////////////////
//  osgBullet, osgWorks libs
//

#ifndef _DEBUG
#pragma comment(lib, "osgwTools.lib")
#if 0
#pragma comment(lib, "osgbDynamics.lib")
#pragma comment(lib, "osgbInteraction.lib")
#pragma comment(lib, "osgbCollision.lib")
#endif
#else 
#pragma comment(lib, "osgwToolsd.lib")
#if 0
#pragma comment(lib, "osgbDynamicsd.lib")
#pragma comment(lib, "osgbInteractiond.lib")
#pragma comment(lib, "osgbCollisiond.lib")
#endif
#endif

#pragma warning(disable:4996)

//////////////////////////////////
// Test and debug defines
//

#define ORIG_EPHEMERIS
#define SCREEN_TEXTURE
// #define DEVELOP_SHADOWS
#define TEST_SHADOWS_FROM_OSG
// #define EXPERIMENTAL_RGB_CAM

/////////////////////////////////
//  Core utils 
//

#include "logger/logger.hpp"
#include "nfi/fn_reg.h"
#include "config/config.h"

#include "cpp_utils/polymorph_ptr.h"
#include "utils/high_res_timer.h"
#include "common/ref_counter.h"

#include <tinyxml2/tinyxml2.h>
#include "xml/tixml_xinclude.h"

/////////////////////////////////
//  CG-Lib
//
#include "common/meta.h"
#include "cg_math.h"

#include "common/points.h"
#include "common/util.h"
#include "geometry/dup_points.h"
#include "geometry/xmath.h"

////////////////////////////////////////////
//  Temp stub 
//

inline cg::geo_base_3 get_base()
{
    return cg::geo_base_3(cg::geo_point_3(0.0,0.0,0));
}

namespace boost {
namespace python {

	class object  
	{
	};
	
	inline static object ptr(const void*)
	{
		return object();
	}

}

}

template<typename T>
inline double get_wheel_radius(T node)
{  
    //const double radius = 0.75 * node->get_bound().radius ;
    return /*0.75 **/ (node->get_bound().size().z / 2.);
}

enum objects_t{
    NONE_TYPE    ,
    AIRCRAFT_TYPE,
    VEHICLE_TYPE
};

#include "nodes_management_fwd.h"

namespace nm = nodes_management;

namespace vehicle
{
	// FIXME for debugging purposes
	struct model_base
	{
		virtual ~model_base             () {};
		virtual void update             ( double /*time*/ ) =0;
		virtual void go_to_pos          ( cg::geo_point_2 pos, double course ) =0;
#ifdef DEPRECATED
		virtual void set_state_debug    (state_t const& state) =0 ;
#endif
		virtual nodes_management::node_info_ptr get_root()=0;
	};

	typedef polymorph_ptr<model_base> model_base_ptr;
	
}

namespace aircraft
{
    inline static   int                    max_desired_velocity() {return 20;};
    inline static   int                    min_desired_velocity() {return 5;};
    inline static   double                 min_radius() {return 18.75;}; 
    inline static   double                 step()       {return 2.0;}; 
}
////////////////////////////////
//  Common includes
//

#include "common/event.h"
#include "kernel/systems/mod_system.h"
#include "phys/phys_sys_fwd.h"

////////////////////////////////////////////
//  Cg to osg and vice versa
//

#include "osg_helpers.h"

////////////////////////////////////////////
//  Common includes (wrong place)
//

#include "impl/local_position.h" // FIXME объекты надо прятать 
#include "fms/trajectory.h" 
#include "objects/aircraft_model_inf.h"
#include "objects/nodes_management.h"



////////////////////////////////////////////
//  av 
//

#include "av/avCore/Database.h"
#include "visitors/find_node_visitor.h"
#include "visitors/visitors.h"
#include "av/avScene.h"


#endif // precompile_header


