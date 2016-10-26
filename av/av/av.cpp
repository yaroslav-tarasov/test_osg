// av.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "av.h"


// This is an example of an exported variable
AV_API int nav=0;

// This is an example of an exported function.
AV_API int fnav(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see av.h for the class definition
Cav::Cav()
{
	return;
}
