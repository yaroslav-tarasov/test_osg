#pragma once

#include "aircraft_view.h"

namespace aircraft
{
	struct ctrl
			: view
	{
		static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        ctrl(object_create_t const& oc, dict_copt dict, optional<cg::geo_point_3> const &initial_pos = boost::none, optional<double> const &initial_course = boost::none);

    protected:
        void update(double time);

    private:
        void set_initial_position(cg::geo_point_3 const &p, double c);

	};
}