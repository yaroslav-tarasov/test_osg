#pragma once

#include "helicopter_physless_view.h"
#include "common/visual_objects_support_fwd.h"
#include "av/avFx/Fx.h"
#include "common/labels_management.h"


namespace helicopter_physless
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
        void on_malfunction_changed  ( aircraft::malfunction_kind_t kind ) override;
        void on_rotor_state           (double target, double speed, rotor_state_t visible) override;
	
		///  labels_management::label_provider_getter
	private:    
		labels_management::labels_provider_ptr      get_label_provider() const;   
    
	private:
        void fill_nodes();

    private:
        nm::node_info_ptr        engine_node_;
        visual_object_ptr        smoke_object_;
        optional<double>         last_update_;
   

    private:
        label_support_proxy_ptr  ls_;
        morphs_support_ptr       ms_;
        smoke_support_ptr        smoke_sup_;

        visual_system*           vsys_;

	private: 
		boost::function<void()>  start_ ;
	    bool                     deffered_init_;
	private:
		static const double sparks_end_duration_;
	};
}            