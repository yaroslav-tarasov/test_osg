#pragma once

#include "aircraft_physless_view.h"

namespace aircraft_physless
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
        void on_malfunction_changed     ( aircraft::malfunction_kind_t kind ) override;

    private:
        nm::node_info_ptr engine_node_;
        visual_object_ptr smoke_object_;
        visual_object_ptr label_object_;
        optional<double>  last_update_;

    private:
        struct label_support;
        boost::shared_ptr<label_support>   ls_;
	};
}