#pragma once

#include "aircraft_view.h"

namespace aircraft
{
	struct ctrl
			: view
            , int_control
	{
		static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

        // int_control
    public:
        void set_trajectory(fms::trajectory_ptr  traj);
        fms::trajectory_ptr  get_trajectory(); 
        decart_position get_local_position();

    private:
        ctrl(object_create_t const& oc, dict_copt dict, optional<cg::geo_point_3> const &initial_pos = boost::none, optional<double> const &initial_course = boost::none);

    protected:
        void update(double time);

    private:
        void set_initial_position(cg::geo_point_3 const &p, double c);

	};
}