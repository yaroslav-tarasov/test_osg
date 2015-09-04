// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef  precompile_header

// time to undef now
#undef SYSTEMS_API

#include "targetver.h"

#include "common/boost.h"
#include "common/stl.h"

#include "common/osg_inc.h"
#include "common/bullet.h"
#include "common/debug.h"

#ifdef QT
#include "common/qt.h"
#endif

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

#pragma warning(disable:4996)


// #define DEVELOP_SHADOWS
#define TEST_SHADOWS_FROM_OSG

#include "nfi/fn_reg.h"

#include "common/meta.h"
#include "cg_math.h"


template<typename T>
inline T atanh (T x)
{
    return (log(1+x) - log(1-x))/2;
}

template<typename T>
inline T cbrt(T n)
{
    if(n>0)
        return std::pow(n, 1/3.);
    else
        return -std::pow(n, 1/3.);
}

namespace cg
{
    template<typename T, typename D>
    __forceinline D lerp_clamp( const T x, const T x0, const T x1, const D & y0, const D & y1 )
    {
        if (x <= x0)
            return y0;
        else if (x < x1)
            return lerp( x0, x1, y0, y1)(x);
        else
            return y1;
    }

    template<typename T> __forceinline T slerp01( const T x )
    {
        return (T(3) - T(2) * x) * x * x;
    };
}



#include "common/points.h"
#include "common/util.h"
#include "geometry/xmath.h"

////////////////////////////////


#include "common/event.h"

#include "atc/position.h"
#include "impl/local_position.h" // FIXME объекты надо прятать 

#include "geometry/dup_points.h"

#include "cpp_utils/polymorph_ptr.h"
#include "objects/nodes_management.h"

namespace nm = nodes_management;
namespace sp = std::placeholders;

inline cg::geo_base_3 get_base()
{
    return cg::geo_base_3(cg::geo_point_3(0.0,0.0,0));
}

#include "fms/trajectory.h"

#include "phys/phys_sys_fwd.h"

#include "objects/aircraft_model_inf.h"
#include "kernel/systems/mod_system.h"

#include "osg_helpers.h"


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


#include "aircraft/aircraft_common.h"
#include "vehicle/vehicle_common.h"
#include "airport/airport_common.h"

namespace vehicle
{
	// FIXME for debugging purposes
	struct model_base
	{
		virtual ~model_base() {};
		virtual void update( double /*time*/ ) =0;
		virtual void on_aerotow_changed(aircraft::info_ptr old_aerotow) =0;
		virtual void go_to_pos(  cg::geo_point_2 pos, double course ) =0;
#ifdef DEPRECATED
		virtual void set_state_debug(state_t const& state) =0 ;
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

#include <tinyxml2/tinyxml2.h>
#include "xml/tixml_xinclude.h"

#include "config/config.h"


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

#include "av/Database.h"
#include "visitors/find_node_visitor.h"

#endif // precompile_header


//#define OSG_NODE_IMPL


