#include "stdafx.h"
#include "precompiled_objects.h"
#include "environment_ctrl.h"
#include "environment_common.h"


namespace environment
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}

AUTO_REG_NAME(environment_ext_ctrl, ctrl::create);

ctrl::ctrl(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, dict)
{
}

void ctrl::settings_edited()
{
    set(msg::settings_msg(settings_)) ;
}

void ctrl::ex_settings_changed()
{
    // double new_start_time = dynamic_cast<app::exercise_document*>(chart_sys_->doc())->get_ex_settings().start_time ;
    double new_start_time = 0;

    if (!cg::eq(settings_.start_time, new_start_time))
        set(msg::start_time(new_start_time)) ;
}

} // environment
