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

#include "common/osg_inc.h"
#include "common/bullet.h"
#include "common/debug.h"

#ifdef QT
#include "common/qt.h"
#endif

#include "common/misc.h"


/////////////////////////////////
//  Core utils 
//

#include "logger/logger.hpp"
#include "nfi/fn_reg.h"
#include "config/config.h"
