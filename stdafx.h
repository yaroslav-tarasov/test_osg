// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef  precompile_header

// time to undef now
#undef SYSTEMS_API

#include "targetver.h"

#ifndef Q_MOC_RUN
#define  BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE
#include "boost/asio.hpp"
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp> 
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <boost/foreach.hpp>

#include <boost/circular_buffer.hpp>

#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <boost/random.hpp>
#include <boost/graph/graphviz.hpp>
                             
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <boost/signals2.hpp>
////////////////////////////////////////////////

using boost::none;

using boost::scoped_ptr;
using boost::scoped_array;
using boost::intrusive_ptr;
using boost::shared_ptr;
using boost::shared_array;
using boost::weak_ptr;
using boost::make_shared;
using boost::enable_shared_from_this;

using boost::optional;
using boost::in_place;

using boost::format;
using boost::wformat;

using boost::bind;
using boost::function;

using boost::any;
using boost::any_cast;
using boost::bad_any_cast;
using boost::static_pointer_cast;

using boost::circular_buffer;
using boost::signals2::scoped_connection;
using boost::signals2::connection;
#endif // Q_MOC_RUN
//////////////////////////////////////
//
//  std include
//
#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <functional>
#include <array>
#include <unordered_map>
#include <memory>
#include <queue>

////////////////////////////////////////////////

using std::deque;
using std::list;
using std::map;
using std::multimap;
using std::queue;
using std::set;
using std::multiset;
using std::stack;
using std::string;
using std::wstring;
using std::vector;
using std::array;
using std::unique_ptr;

using std::ofstream;
using std::ifstream;

using std::exception;
using std::runtime_error;

using std::pair;
using std::make_pair;

using std::move;
using std::forward;




//////////////////////////////////////
//
//  osg include
//
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Point>

#include <osg/BlendFunc>

#include <osg/PatchParameter>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/FirstPersonManipulator>
#include <osgGA/SphericalManipulator>
#include <osgGA/OrbitManipulator>
#include <osgGA/TerrainManipulator>

#include <osgSim/OverlayNode>

#include <osgViewer/Viewer>
#include <osgViewer/config/SingleScreen>

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/TimelineAnimationManager>

#include <osg/AnimationPath>
#include <osg/PositionAttitudeTransform>  

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include <osgParticle/PrecipitationEffect>

#include <osgFX/Outline>

#include <osg/Depth>
                 
#include <osgUtil/CullVisitor> 

#include <osg/TextureCubeMap>
#include <osg/Transform>

#include <osgViewer/ViewerEventHandlers>

#include <osg/StateSet>
#include <osg/LightModel>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/ShadowTexture>
#include <osgShadow/ViewDependentShadowMap>

#include <osgGA/NodeTrackerManipulator>
#include <osgUtil/LineSegmentIntersector>
#include <osg/io_utils>

#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgUtil/PerlinNoise>
#include <osg/TriangleFunctor>
#include <osg/ImageSequence>
#include <osgFX/BumpMapping>
#include <osgUtil/TangentSpaceGenerator>

#include <osg/AlphaFunc>
#include <osg/BlendEquation>

#include <osg/TexGen>
#include <osg/TexEnvCombine>

#include <osgUtil/ReflectionMapGenerator>
#include <osgUtil/HighlightMapGenerator>
#include <osgUtil/HalfWayMapGenerator>
#include <osgUtil/Simplifier>

#include <osg/ComputeBoundsVisitor>
#include <osg/CameraNode>
#include <osg/FrontFace>
#include <osg/CullFace>

#include <osg/ValueObject>


#define GL_SAMPLE_ALPHA_TO_COVERAGE      0x809E
#define GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB 0x884F

#define BASE_SHADOW_TEXTURE_UNIT 6

#define STRINGIFY(x) #x 

#ifdef _DEBUG
    #pragma comment(lib, "osgTextd.lib")
    #pragma comment(lib, "osgShadowd.lib")
    #pragma comment(lib, "osgFXd.lib")
    #pragma comment(lib, "osgEphemerisd.lib")
    #pragma comment(lib, "OpenThreadsd.lib")
    #pragma comment(lib, "osgd.lib")
    #pragma comment(lib, "osgDBd.lib")
    #pragma comment(lib, "osgViewerd.lib")
    #pragma comment(lib, "osgAnimationd.lib")
    #pragma comment(lib, "osgGAd.lib")
    #pragma comment(lib, "osgUtild.lib")
    #pragma comment(lib, "osgSimd.lib")
    #pragma comment(lib, "osgParticled.lib")
    #pragma comment(lib, "BulletCollision_Debug.lib")
    #pragma comment(lib, "LinearMath_Debug.lib")
    #pragma comment(lib, "BulletDynamics_Debug.lib")
#else 
    #pragma comment(lib, "osgText.lib")
    #pragma comment(lib, "osgShadow.lib")
    #pragma comment(lib, "osgFX.lib")
    #pragma comment(lib, "osgEphemeris.lib")
    #pragma comment(lib, "OpenThreads.lib")
    #pragma comment(lib, "osg.lib")
    #pragma comment(lib, "osgDB.lib")
    #pragma comment(lib, "osgViewer.lib")
    #pragma comment(lib, "osgAnimation.lib")
    #pragma comment(lib, "osgGA.lib")
    #pragma comment(lib, "osgUtil.lib")
    #pragma comment(lib, "osgSim.lib")
    #pragma comment(lib, "osgParticle.lib")
    #pragma comment(lib, "BulletCollision.lib")
    #pragma comment(lib, "LinearMath.lib")
    #pragma comment(lib, "BulletDynamics.lib")
#endif

#ifndef _DEBUG
#pragma comment(lib, "SPARK_GL.lib")
#pragma comment(lib, "SPARK.lib")
#else 
#pragma comment(lib, "SPARK_GL_debug.lib")
#pragma comment(lib, "SPARK_debug.lib")
#endif

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


#ifdef _DEBUG
#pragma comment(lib, "boost_thread-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-gd-1_50.lib")
#else
#pragma comment(lib, "boost_thread-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-1_50.lib")
#endif

// #define DEVELOP_SHADOWS
#define TEST_SHADOWS_FROM_OSG

using boost::noncopyable;
using boost::optional;

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

#define Assert(x) if(x){};

#include "common/points.h"

#include "common/util.h"

#include "geometry/xmath.h"
//#include "geometry/primitives.h"
////////////////////////////////
//
//
//using cg::geo_point_3;
//using cg::geo_point_2;
//using cg::point_2;
//using cg::point_2f;
//using cg::point_3;
//using cg::point_3f;
//using cg::point_3i;
//using cg::cpr;
//using cg::quaternion;
//using cg::transform_4;             
//using cg::geo_base_3;
//////////////////////////////////

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

#include "trajectory.h"

#include "phys/phys_sys_fwd.h"
#include "aircraft/aircraft_common.h"  // FIXME объекты надо прятать 

#include "objects/aircraft_model_inf.h"
#include "mod_system.h"

#include "osg_helpers.h"
#include "visitors/find_node_visitor.h"

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

#include "vehicle/vehicle_common.h"

namespace vehicle
{
	// FIXME just stub
	struct model_base
	{
		virtual ~model_base() {};
		virtual void update( double /*time*/ ) =0;
		virtual void on_aerotow_changed(aircraft::info_ptr old_aerotow) =0;
		virtual void go_to_pos(  cg::geo_point_2 pos, double course ) =0;
		virtual void set_state_debug(state_t const& state) =0 ;
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

#define STR(x) STRINGIFY(x)
#define FIXME(x) __pragma(message(__FILE__ "(" STR(__LINE__) "): " "fixme: " STRINGIFY(x) ))

#include <tinyxml2/tinyxml2.h>
#include "xml/tixml_xinclude.h"

#endif // precompile_header



