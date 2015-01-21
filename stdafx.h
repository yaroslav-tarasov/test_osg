// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

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

#include <osg/ComputeBoundsVisitor>
#include <osg/CameraNode>
#include <osg/FrontFace>
#include <osg/CullFace>




int main_scene( int argc, char** argv );
int main_hud( int argc, char** argv );
int main_select( int argc, char** argv );
int main_shadows(int argc, char *argv[]);
int main_shadows_2( int argc, char** argv );
int main_shadows_3(int argc, char** argv);
int main_texturedGeometry(int argc, char** argv);
int main_TestState(int argc, char** argv);
int main_tess_test( int argc, char** argv );
int main_tex_test( int argc, char** argv );
int main_bump_map( int argc, char** argv );
int main_exp_test( int argc, char** argv );
int main_bi( int argc, char** argv );
int main_teapot( int argc, char** argv );
int main_spark( int argc, char** argv );
int main_scene2( int argc, char** argv );

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

// typedef osg::Quat    quaternion;
// typedef osg::Vec3  geo_base_3;
// typedef osg::Vec3  geo_position;
// typedef osg::Vec3  geo_point_3;
// typedef osg::Vec2    point_2;
// typedef osg::Vec3    point_3;
// typedef osg::Matrix  transform_4;


#include "Windows.h"
#undef min
#undef max

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp> 
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/math/constants/constants.hpp>

using boost::noncopyable;
using boost::optional;

// #define DEVELOP_SHADOWS
#define TEST_SHADOWS_FROM_OSG

#include "meta.h"
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

#include "cpp_utils/polymorph_ptr.h"
#include "nodes_management.h"
namespace nm = nodes_management;