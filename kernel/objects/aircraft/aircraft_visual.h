#pragma once

#include "aircraft_view.h"

namespace aircraft
{
	struct visual
			: view
	{
		static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        visual(object_create_t const& oc, dict_copt dict);

    protected:
        void update(double time);

    private:
        void on_malfunction_changed     ( malfunction_kind_t kind ) override;
        void on_engine_state_changed    ( engine_state_t state ) override;

    private:
        nm::node_info_ptr engine_node_;
        visual_object_ptr smoke_object_;
        visual_object_ptr label_object_;
        optional<double>  last_update_;
	};
}