#pragma once

//#include "common/flock_child.h"

namespace flock
{
	namespace manager
	{
		struct info
		{
			virtual geo_point_3 const&     pos() const =0;
			virtual const settings_t& settings() const =0;
			virtual ~info() {}
		};

		typedef polymorph_ptr<info> info_ptr;
	}

}