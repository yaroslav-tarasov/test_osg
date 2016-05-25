#pragma once

#include "aircraft_physless_view.h"
#include "av/avFx/Fx.h"

namespace visual_objects
{
    struct label_support;
    struct parashute_support;
}

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
        void on_malfunction_changed    ( aircraft::malfunction_kind_t kind ) override;
        void on_new_wheel_contact_effect(double time, point_3f vel, point_3f offset) override;

    private:
        void fill_nodes();
    
    private:
        nm::node_info_ptr engine_node_;
        visual_object_ptr smoke_object_;
        visual_object_ptr label_object_;
        optional<double>  last_update_;

		struct smoke_sfx_t
		{
			smoke_sfx_t(visual_system*    vsys, visual*    vthis)
				: vsys (vsys)
				, vthis_(vthis)
			{}
			
			visual_object_ptr    smoke_object_;
			SmokeSfxNode * smoke_sfx_weak_ptr_;

			double last_fire_time_;
			double   smoke_factor_;
			visual_system*    vsys;


			void on_malfunction_changed( aircraft::malfunction_kind_t kind );
		
		private:
			visual*          vthis_;
		};

		smoke_sfx_t        smoke_sfx_;
        
        visual_object_ptr    landing_dust_object_;
        LandingDustSfxNode * landing_dust_weak_ptr_;

    private:
        boost::shared_ptr<visual_objects::label_support>       ls_;
        boost::shared_ptr<visual_objects::parashute_support>   ps_;

	private: 
		boost::function<void()>                       start_ ;
	
	private:
		static const double smoke_end_duration_;
		static const double sparks_end_duration_;
	};
}