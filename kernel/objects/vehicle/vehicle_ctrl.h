#pragma once

#include "vehicle_view.h"

namespace vehicle
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
    private:
        void attach_tow() override; 
        void detach_tow() override; 
	};
}