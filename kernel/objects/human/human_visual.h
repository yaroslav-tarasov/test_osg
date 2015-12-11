#pragma once 
#include "human_view.h"
#include "network/msg_dispatcher.h"
#include "objects/impl/local_position.h"
#include "kernel/systems/vis_system.h"

namespace human
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
        optional<visual_object_ptr>      tow_visual_object_;
        nodes_management::node_info_ptr  aero_tow_point_; 
        // experimental part
    private:
        struct tow_support;
        boost::shared_ptr<tow_support>   ts_;

    };

}