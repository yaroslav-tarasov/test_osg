// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

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

#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <boost/random.hpp>
#include <boost/graph/graphviz.hpp>
                             
#include <boost/property_tree/ptree.hpp>




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

//int main_scene( int argc, char** argv );
//int main_hud( int argc, char** argv );
//int main_select( int argc, char** argv );
//int main_shadows(int argc, char *argv[]);
//int main_shadows_2( int argc, char** argv );
//int main_shadows_3(int argc, char** argv);
//int main_texturedGeometry(int argc, char** argv);
//int main_TestState(int argc, char** argv);
//int main_tess_test( int argc, char** argv );
//int main_tex_test( int argc, char** argv );
//int main_bump_map( int argc, char** argv );
//int main_exp_test( int argc, char** argv );
//int main_bi( int argc, char** argv );
//int main_teapot( int argc, char** argv );
//int main_spark( int argc, char** argv );
//int main_scene2( int argc, char** argv );
//int main_dubins( int argc, char** argv );
//int main_net(int argc, char** argv);

#define GL_SAMPLE_ALPHA_TO_COVERAGE      0x809E
#define GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB 0x884F

#define BASE_SHADOW_TEXTURE_UNIT 6

#define STRINGIFY(x) #x 

#ifdef _DEBUG
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


#pragma warning(disable:4996)

#include "Windows.h"
#undef min
#undef max



#ifdef _DEBUG
#pragma comment(lib, "boost_thread-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-gd-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-gd-1_50.lib")
#else
#pragma comment(lib, "boost_thread-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_filesystem-vc100-mt-1_50.lib")
#pragma comment(lib, "boost_system-vc100-mt-1_50.lib")
#endif
  

using boost::noncopyable;
using boost::optional;

//
//  refl stubs
//

//#define REFL_INNER(x)
//#define REFL_SER_BIN(x)
//#define REFL_ENTRY(x)
//#define REFL_END()
//#define REFL_STRUCT(x)
//#define REFL_NUM(x1,x2,x3,x4)

// #define DEVELOP_SHADOWS
#define TEST_SHADOWS_FROM_OSG

#include "common/meta.h"
#include "cg_math.h"
#include "geometry/xmath.h"

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

#include "geometry/primitives.h"
#include "position.h"
#include "local_position.h"

namespace binary
{
    // size
    typedef uint32_t size_type;
}

#include "geometry/dup_points.h"

#include "cpp_utils/polymorph_ptr.h"
#include "nodes_management.h"

namespace nm = nodes_management;
namespace sp = std::placeholders;

inline cg::geo_base_3 get_base()
{
    return cg::geo_base_3(cg::geo_point_3(0.0,0.0,0));
}

#include "trajectory.h"

#include "phys_sys_fwd.h"
#include "aircraft_common.h"

#include "aircraft_model_inf.h"

#include "event.h"
#include "mod_system.h"

#include "osg_helpers.h"

#include "cpp_utils/func_pointer.h"

namespace fn_reg
{

typedef
    boost::unordered_map<std::string, boost::any>
    func_collection_t;

inline func_collection_t& func_collection()
{
    static func_collection_t collection;
    return collection;
}

//! получение указателя на синглтон коллекции функций
inline func_collection_t const* extract_collection()
{
    return &(func_collection());
}

//! глобальный регистратор; в конструкторе обращение к синглтону в котором регистрируем объект
struct func_registrator
{
    template<typename function_type>
    func_registrator(const char* name, function_type func)
    {
        func_collection()[name] = func;
    }
};


inline  boost::any extract_function(std::string const& function_name)
{
   func_collection_t const& col = func_collection();
    auto it = col.find(function_name);

    if (it == col.end())
        return boost::any();

    return it->second;
}

template<typename signature>
typename cpp_utils::func_pointer<signature>::type
    function( std::string const& function_name)
{
    // FIXME TODO
    // DECL_LOGGER("nfi");

    boost::any func = extract_function( function_name);

    if (func.empty())
    {
        return nullptr;
    }

    try
    {
        return boost::any_cast<typename cpp_utils::func_pointer<signature>::type>(func);
    }
    catch(boost::bad_any_cast const&)
    {
        // FIXME TODO
        // LogError("function type mismatch: " << function_name << " in " << lib_name);
    }

    return nullptr;
}

}

#define AUTO_REG_NAME_IMPL(name, func)          \
    namespace                                       \
{                                               \
    fn_reg::func_registrator              \
    __registrator__##name(#name, &func);    \
}

#define AUTO_REG_NAME(name, func)   AUTO_REG_NAME_IMPL(name, func)
#define AUTO_REG(func)              AUTO_REG_NAME_IMPL(func, func)

