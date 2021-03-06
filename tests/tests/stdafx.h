// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//////////////////////////////////
//  Common includes
//

#include "common/boost.h"
#include "common/stl.h"

#include "osgnanovg/precompile_nvg.h"

#include "common/osg_inc.h"
#include "common/bullet.h"
#include "common/debug.h"

#ifdef QT
#include "common/qt.h"
#endif

#include "common/av_misc.h"


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
#include "common/av_math.h"

#include "common/points.h"
#include "common/util.h"
#include "geometry/dup_points.h"
#include "geometry/xmath.h"

////////////////////////////////////////////

enum objects_t{
    NONE_TYPE    ,
    AIRCRAFT_TYPE,
    VEHICLE_TYPE
};

////////////////////////////////
//  Common includes
//

#include "common/event.h"

////////////////////////////////////////////
//  Common includes (wrong place)
//

#include "impl/local_position.h" // FIXME ������� ���� ������� 
#include "fms/trajectory.h" 

////////////////////////////////////////////
//  av 
//

#include "av/avCore/Database.h"
#include "av/avUtils/visitors/find_node_visitor.h"
#include "av/avUtils/visitors/visitors.h"
#include "av/avScene.h"

#include "nodes_management_fwd.h"

namespace nm = nodes_management;

namespace utils
{
	const osg::Vec4 red_color   (100.0f, 0.0f, 0.0f, 100.0f);
	const osg::Vec4 blue_color  (0.0f, 0.0f, 100.0f, 100.0f);
	const osg::Vec4 green_color (0.0f, 100.0f, 0.0f, 100.0f);
	const osg::Vec4 white_color (100.0f, 100.0f, 100.0f, 100.0f);
	const osg::Vec4 black_color (0.0f,0.0f,0.0f,100.0f);
	const osg::Vec4 gray_color  (0.8f,0.8f,0.8f,100.0f);
}
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBody.h>

#include "btBulletWorldImporter.h"
#include <btBulletFile.h>