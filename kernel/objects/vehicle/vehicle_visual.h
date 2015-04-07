#pragma once 
#include "vehicle_view.h"
#include "network/msg_dispatcher.h"
#include "objects/impl/local_position.h"
#include "kernel/systems/vis_system.h"

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
        optional<visual_object_ptr>      tow_visual_object_;
        nodes_management::node_info_ptr  aero_tow_point_; 
        // experimental part
    private:
        double                           radius_;
        double                           radius_s_;
        double                           radius_a_;
        double                           radius_b_;
        osg::Node *                      body_s_;
        osg::Node *                      body_a_;
        osg::Node *                      body_b_;

    };

}