#pragma once 
#include "camera_view.h"
#include "network/msg_dispatcher.h"

namespace camera_object
{
	struct ctrl
		: visual_presentation
		, visual_control
        , control
		, view
	{
		static object_info_ptr create(object_create_t const& oc, dict_copt dict);

	private:
		ctrl(object_create_t const& oc, dict_copt dict);

	private:
		fms::trajectory_ptr  get_trajectory();
		void set_trajectory(fms::trajectory_ptr  traj);
        void set_trajectory(const fms::traj_data&  traj_data);

		// visual_control
	private:
		geo_point_3 pos  () const override;
		cpr         orien() const override;

	private:
		double magnification_;

	};

}