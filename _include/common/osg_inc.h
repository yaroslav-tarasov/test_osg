#pragma once

//////////////////////////////////////
//
//  osg include
//
#include <osg/io_utils>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Point>
#include <osg/BlendFunc>
#include <osg/PatchParameter>
#include <osg/TextureCubeMap>


#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TriangleFunctor>
#include <osg/ImageSequence>
#include <osg/AnimationPath>
#include <osg/Transform>
#include <osg/PositionAttitudeTransform>
#include <osg/AlphaFunc>
#include <osg/BlendEquation>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/TexEnvCombine>
#include <osg/StateSet>
#include <osg/LightModel>
#include <osg/ComputeBoundsVisitor>
#include <osg/CameraNode>
#include <osg/FrontFace>
#include <osg/CullFace>
#include <osg/ValueObject>
#include <osg/Depth>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/PluginQuery>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/FirstPersonManipulator>
#include <osgGA/SphericalManipulator>
#include <osgGA/OrbitManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/NodeTrackerManipulator>

#include <osgSim/OverlayNode>

#include <osgViewer/Viewer>
#include <osgViewer/config/SingleScreen>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Version>

#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/TimelineAnimationManager>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/PrecipitationEffect>

#include <osgFX/Outline>

#include <osgUtil/CullVisitor> 
#include <osgUtil/Optimizer>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/TangentSpaceGenerator>
#include <osgUtil/PerlinNoise>
#include <osgUtil/ReflectionMapGenerator>
#include <osgUtil/HighlightMapGenerator>
#include <osgUtil/HalfWayMapGenerator>
#include <osgUtil/Simplifier>
#include <osgUtil/SmoothingVisitor>


#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/ShadowTexture>
#include <osgShadow/ViewDependentShadowMap>

#include <osgFX/BumpMapping>

#include <osgSim/LightPointNode>

#define GL_SAMPLE_ALPHA_TO_COVERAGE      0x809E
#define GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB 0x884F


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
#pragma comment(lib, "BulletWorldImporter_Debug.lib")
#pragma comment(lib, "BulletFileLoader_Debug.lib")
#else
#ifdef OSG_USE_MINREL
#pragma comment(lib, "osgTexts.lib")
#pragma comment(lib, "osgShadows.lib")
#pragma comment(lib, "osgFXs.lib")
#pragma comment(lib, "osgEphemeriss.lib")
#pragma comment(lib, "OpenThreadss.lib")
#pragma comment(lib, "osgs.lib")
#pragma comment(lib, "osgDBs.lib")
#pragma comment(lib, "osgViewers.lib")
#pragma comment(lib, "osgAnimations.lib")
#pragma comment(lib, "osgGAs.lib")
#pragma comment(lib, "osgUtils.lib")
#pragma comment(lib, "osgSims.lib")
#pragma comment(lib, "osgParticles.lib")
#pragma comment(lib, "BulletCollision.lib")
#pragma comment(lib, "LinearMath.lib")
#pragma comment(lib, "BulletDynamics.lib")   
#pragma comment(lib, "BulletWorldImporter.lib")
#pragma comment(lib, "BulletFileLoader.lib")
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
#pragma comment(lib, "BulletWorldImporter.lib")
#pragma comment(lib, "BulletFileLoader.lib")
#endif
#endif