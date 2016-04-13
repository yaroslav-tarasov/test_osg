#pragma once 
#include "camera_view.h"
#include "network/msg_dispatcher.h"

namespace camera_object
{
	struct ctrl
		: visual_presentation
		, visual_control
		, view
	{
		static object_info_ptr create(object_create_t const& oc, dict_copt dict);

	private:
		ctrl(object_create_t const& oc, dict_copt dict);

		// visual_control
	private:
		geo_point_3 pos  () const override;
		cpr         orien() const override;

	private:
		double magnification_;

	};

}