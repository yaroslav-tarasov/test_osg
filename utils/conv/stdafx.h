// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include "common/boost.h"
#include "common/stl.h"
#include "common/osg_inc.h"
#include "common/misc.h"
#include "common/bullet.h"

#include "cg_math.h"

#include "common/points.h"

#include "common/util.h"
#include "common/debug.h"

#include "geometry/xmath.h"

bool generateBulletFile(std::string name, osg::Node* body, cg::point_3& offset);


#include "core/nfi/fn_reg.h"
#include "core/config/config.h"

#include "av/avCore/Database.h"

#include "cpp_utils/polymorph_ptr.h"
#include "common/ref_counter.h"
#include "av/avScene.h"

namespace sp = std::placeholders;