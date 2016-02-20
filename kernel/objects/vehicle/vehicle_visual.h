#pragma once 
#include "vehicle_view.h"
#include "network/msg_dispatcher.h"
#include "objects/impl/local_position.h"
#include "kernel/systems/vis_system.h"

#include "av/avFx/Fx.h"

namespace visual_objects
{
    struct label_support;
}

namespace vehicle
{
    struct visual
        : view 
    {
        static object_info_ptr create(object_create_t const& oc, dict_copt dict);

    private:
        visual(object_create_t const& oc, dict_copt dict);

    protected:
        void update     (double time);

    private:
        void settings_changed   ()             override;
    private:
        optional<visual_object_ptr>      tow_visual_object_;
        nodes_management::node_info_ptr  aero_tow_point_; 
        // experimental part
    private:
        struct tow_support;
        boost::shared_ptr<tow_support>   ts_;

    private:
        visual_object_ptr                       label_object_;
		FoamStreamSfxNode *         foam_stream_sfx_weak_ptr_;
		visual_object_ptr                 foam_stream_object_;
    private:
        boost::shared_ptr<visual_objects::label_support>   ls_;

	private:
		double                  last_fs_time_;
		double                  fs_factor_;
        boost::optional<double> fs_start_time_;
        boost::optional<double> fs_stop_time_;
	private:
		static const double     fs_end_duration_;
    };

}