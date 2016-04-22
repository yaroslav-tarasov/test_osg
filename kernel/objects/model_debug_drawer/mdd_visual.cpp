#include "stdafx.h"
#include "precompiled_objects.h"

#include "mdd_visual.h"

namespace mdd
{
    object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
    {
        return object_info_ptr(new visual(oc,dict));
    }

    visual::visual(kernel::object_create_t const& oc, dict_copt dict)
        : view(oc, dict)
    {   
        msg_disp()
            .add<msg::state_msg_t               >(boost::bind(&visual::on_state               , this, _1))
            .add<msg::draw_line_msg_t           >(boost::bind(&visual::on_draw_line           , this, _1))
            .add<msg::draw_sphere_msg_t         >(boost::bind(&visual::on_draw_sphere         , this, _1))
            .add<msg::draw_triangle_msg_t       >(boost::bind(&visual::on_draw_triangle       , this, _1))
            .add<msg::draw_contact_point_msg_t  >(boost::bind(&visual::on_draw_contact_point  , this, _1))
            ;
    }

    void visual::update( double /*time*/ )
    {
    }

    debug_render_ptr visual::get_renderer()
    {
        return renderer_;
    }

    void visual::on_state              (msg::state_msg_t const& msg)
    {

    }

    void visual::on_draw_line          (msg::draw_line_msg_t const& msg)
    {

    }

    void visual::on_draw_sphere        (msg::draw_sphere_msg_t const& msg)
    {

    }

    void visual::on_draw_triangle      (msg::draw_triangle_msg_t const& msg)
    {

    }

    void visual::on_draw_contact_point (msg::draw_contact_point_msg_t const& msg)
    {

    }


    AUTO_REG_NAME(mdd_visual, visual::create);

}