// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include "common/osg_inc.h"
#include "common/bullet.h"

//////////////////////////////////
//  osgBullet, osgWorks libs
//


#ifndef _DEBUG
#pragma comment(lib, "osgwTools.lib")
#else 
#pragma comment(lib, "osgwToolsd.lib")
#endif



