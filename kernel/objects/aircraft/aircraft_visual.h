#pragma once

#include "aircraft_view.h"
#include "common/visual_objects_support_fwd.h"
#include "common/labels_management.h"

namespace aircraft
{
	using namespace visual_objects;

	struct visual
			: view
            , labels_management::label_provider_getter
	{
		static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        visual(object_create_t const& oc, dict_copt dict);

    protected:
        void update(double time);

    private:
        void on_malfunction_changed     ( malfunction_kind_t kind ) override;
        void on_equipment_state_changed ( equipment_state_t state ) override;
		
		///  labels_management::label_provider_getter
	private:    
		labels_management::labels_provider_ptr      get_label_provider() const;

    private:
        nm::node_info_ptr engine_node_;
        visual_object_ptr smoke_object_;
        
		label_support_proxy_ptr  ls_;
		
		visual_system*           vsys_;

        optional<double>  last_update_;
	};
}