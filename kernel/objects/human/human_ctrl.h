#pragma once

#include "human_view.h"

namespace human
{
	struct ctrl
			: view
            , control
	{
		static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        ctrl(object_create_t const& oc, dict_copt dict);

    protected:
        void update(double time);

    private:
        void set_initial_position(cg::geo_point_3 const &p, double c);
        void goto_pos(geo_point_2 pos,double course) override;
        void follow_route(const std::string& name)   override;

        void follow_trajectory(std::string const& /*route*/);
        decart_position      get_local_position() const;
        fms::trajectory_ptr  get_trajectory();
        void set_trajectory(fms::trajectory_ptr  traj);
    private:
        void attach_tow() override; 
        void detach_tow() override; 
        void set_brake(double val) override;
	};
}