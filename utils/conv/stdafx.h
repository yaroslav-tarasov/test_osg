// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define BOOST_ALL_NO_LIB

#include "common/boost.h"
#include "common/stl.h"
#include "common/osg_inc.h"

#include "cg_math.h"

#define Assert(x) if(x){};

#include "common/points.h"

#include "common/util.h"

#include "geometry/xmath.h"

#define STRINGIFY(x) #x 

#define STR(x) STRINGIFY(x)
#define FIXME(x) __pragma(message(__FILE__ "(" STR(__LINE__) "): " "fixme: " STRINGIFY(x) ))

bool generateBulletFile(std::string name, osg::Node* body, cg::point_3& offset);
