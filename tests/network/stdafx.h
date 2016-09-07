// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#include "common/boost.h"
#include "common/stl.h"

#include "reflection/proc/binary.h"
#include "common/points.h"

#include "atc/position.h"

#include "cpp_utils/polymorph_ptr.h"

inline cg::geo_base_3 get_base()
{
    return cg::geo_base_3(cg::geo_point_3(0.0,0.0,0));
}

#include "common/debug.h"
#include "fms/trajectory.h"

namespace sp = std::placeholders;