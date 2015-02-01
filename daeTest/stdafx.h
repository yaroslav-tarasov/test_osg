// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4180 4245)
#endif
#include <boost/filesystem/convenience.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <list>
#include <vector>
#include <iostream>
#include <dae.h>
#include <dom/domElements.h>
#include <dom/domMaterial.h>
#include <dom/domGeometry.h>
#include <dom/domNode.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>
#include <dom/domProfile_COMMON.h>


#include "pugixml.hpp"


#ifdef _DEBUG
	#pragma comment(lib, "libboost_filesystem-vc100-mt-gd-1_50.lib")
	#pragma comment(lib, "libboost_system-vc100-mt-gd-1_50.lib")
#else
	#pragma comment(lib, "libboost_filesystem-vc100-mt-1_50.lib")
	#pragma comment(lib, "libboost_system-vc100-mt-1_50.lib")
#endif



