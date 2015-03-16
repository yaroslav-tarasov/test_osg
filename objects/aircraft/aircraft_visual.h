#pragma once

#include "aircraft_view.h"

namespace aircraft
{
	struct vis
			: view
	{
		static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

		vis( kernel::object_create_t const& oc, dict_copt dict );
	};
}