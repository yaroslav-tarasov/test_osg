// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <functional>
#include <memory>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/ProxyNode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Timer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/PluginQuery>

#include <osgUtil/Optimizer>
#include <osgUtil/Simplifier>
#include <osgUtil/SmoothingVisitor>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Version>
#include <osg/MatrixTransform>
#include <osgUtil/Optimizer>

#include <iostream>


#ifndef _DEBUG
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
#else 
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
//#pragma comment(lib, "BulletCollision_Debug.lib")
//#pragma comment(lib, "LinearMath_Debug.lib")
//#pragma comment(lib, "BulletDynamics_Debug.lib")
#endif


#include <boost/algorithm/string.hpp>