#pragma once

//#include "common/flock_child.h"

namespace flock
{
	namespace manager
	{
		struct info
		{
			virtual const settings_t& settings() const =0;
			virtual ~info() {}
		};

		typedef polymorph_ptr<info> info_ptr;
	}

}