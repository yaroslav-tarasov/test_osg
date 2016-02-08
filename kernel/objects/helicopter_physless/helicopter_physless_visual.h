#pragma once

#include "helicopter_physless_view.h"
#include "av/avFx/Fx.h"

namespace visual_objects
{
    struct label_support;
}

namespace helicopter_physless
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
        void on_malfunction_changed  ( aircraft::malfunction_kind_t kind ) override;
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

        void fill_nodes();
    private:
        boost::shared_ptr<visual_objects::label_support>   ls_;

	private: 
		boost::function<void()>                       start_ ;
	    bool                                          deffered_init_;
	private:
		static const double smoke_end_duration_;
		static const double sparks_end_duration_;
	};
}