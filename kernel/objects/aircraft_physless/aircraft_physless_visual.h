#pragma once

#include "aircraft_physless_view.h"
#include "common/aircraft_support_fwd.h"

#include "av/avFx/Fx.h"



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
        void on_new_wheel_contact_effect( double time, point_3f vel, point_3f offset ) override;
        void on_equipment_state_changed ( aircraft::equipment_state_t state ) override; 

    private:
        void fill_nodes();
    
    private:
        optional<double>  last_update_;

        nm::node_info_ptr engine_node_;
        nm::node_info_ptr forsage_node_;

        visual_object_ptr smoke_object_;
        
#if 0
        visual_object_ptr    landing_dust_object_;
        LandingDustSfxNode * landing_dust_weak_ptr_;
#endif

    private:
        boost::shared_ptr<visual_objects::label_support>       ls_;
        boost::shared_ptr<visual_objects::parashute_support>   ps_;
        boost::shared_ptr<visual_objects::forsage_support>     fs_;
        boost::shared_ptr<visual_objects::smoke_support>       smoke_sup_;
        boost::shared_ptr<visual_objects::landing_dust_support>  lds_;

        visual_system*                                         vsys_;

	private: 
		boost::function<void()>                                start_ ;
	
	private:
		static const double sparks_end_duration_;
	};
}