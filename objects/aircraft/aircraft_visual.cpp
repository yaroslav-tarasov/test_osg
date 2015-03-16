#include "stdafx.h"
#include "precompiled_objects.h"
#include "aircraft_visual.h"

namespace aircraft
{

	object_info_ptr vis::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new vis(oc, dict));
	}

	AUTO_REG_NAME(aircraft_visual, vis::create);

	vis::vis( kernel::object_create_t const& oc, dict_copt dict )
		: view(oc,dict)
	{

	}


}


