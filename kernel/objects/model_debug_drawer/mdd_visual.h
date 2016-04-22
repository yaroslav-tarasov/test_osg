#pragma once

#include "mdd_view.h"
#include "common/model_debug_drawer.h"

namespace mdd
{

    struct visual
        : view
        , info
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        visual(kernel::object_create_t const& oc, dict_copt dict);

        // base_presentation
    private:
        void update( double /*time*/ );

        // info
    private:
        debug_render_ptr get_renderer();

        // msgs
    private:
        void on_state              (msg::state_msg_t const& msg);
        void on_draw_line          (msg::draw_line_msg_t const& msg);
        void on_draw_sphere        (msg::draw_sphere_msg_t const& msg);
        void on_draw_triangle      (msg::draw_triangle_msg_t const& msg);
        void on_draw_contact_point (msg::draw_contact_point_msg_t const& msg);

    private:
        void render();

    private:
        debug_render_ptr renderer_;
    };

}