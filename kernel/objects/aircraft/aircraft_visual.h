#pragma once

#include "aircraft_view.h"
#include "common/visual_objects_support_fwd.h"
#include "common/labels_management.h"

#include "av/avFx/Fx.h"

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
        void on_new_wheel_contact_effect( double time, point_3f vel, point_3f offset ) override;
        void on_equipment_state_changed ( equipment_state_t state ) override;

    private:
        void fill_nodes();
		
        ///  labels_management::label_provider_getter
	private:    
		labels_management::labels_provider_ptr      get_label_provider() const;

    private:
        optional<double>                           last_update_;

        std::vector<nm::node_info_ptr>             engines_nodes_;

        std::vector<nm::node_info_ptr>             forsage_nodes_;
        std::vector<forsage_support_ptr>           fs_;

        visual_object_ptr smoke_object_;

    private:
        label_support_proxy_ptr                                 ls_;

        std::vector<nm::node_info_ptr>             parachute_nodes_;
        std::vector<parashute_support_ptr>                      ps_;

        smoke_support_ptr                                smoke_sup_;
        landing_dust_support_ptr                               lds_;

        visual_system*                                        vsys_;

    private: 
        boost::function<void()>                             start_ ;

    private:
        static const double sparks_end_duration_;

	};
}