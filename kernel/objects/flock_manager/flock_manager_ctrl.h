#pragma once

#include "flock_manager_view.h"

namespace flock
{

	namespace manager
	{

		struct ctrl
			: view 
		{
			static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

		protected:
			ctrl(kernel::object_create_t const& oc, dict_copt dict);

		};


	}

}
