#include "stdafx.h"
#include "precompiled_objects.h"

#include "environment_view.h"
#include "environment_common.h"
#include "kernel/object_info.h"

namespace environment
{
object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(environment_view, view::create);

    view::view( kernel::object_create_t const& oc, dict_copt dict )
        : base_view_presentation(oc)
        , obj_data_base         (dict)
    {
        msg_disp()
            .add<msg::settings_msg>(boost::bind(&view::on_settings  , this, _1))
            .add<msg::start_time  >(boost::bind(&view::on_start_time, this, _1));
    }

void view::on_settings(msg::settings_msg const& m)
{
    settings_ = m.settings;
    on_settings_changed();
}

void view::on_start_time(msg::start_time const& m)
{
    settings_.start_time = m.time;
    on_start_time_changed();
}

void view::set_new_start_time(double new_start_time)
{
    if (!cg::eq(settings_.start_time, new_start_time))
        set(msg::start_time(new_start_time));
}

} // environment


