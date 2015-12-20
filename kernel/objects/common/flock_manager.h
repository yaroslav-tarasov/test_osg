#pragma once

#include "common/flock_child.h"

namespace flock
{
	namespace manager
	{
		struct info
		{
			virtual ~info() {}
		};

		typedef polymorph_ptr<info> info_ptr;
	}

}