#pragma once

#include "common/environment.h"
#include "environment/environment_common.h"

namespace environment
{
    struct view
        : base_view_presentation
        , obj_data_holder<wrap_settings<settings_t>>
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    protected:
        view( kernel::object_create_t const& oc, dict_copt dict );

    public:
        void on_settings  (msg::settings_msg const& msg);
        void on_start_time(msg::start_time   const& msg);

    protected:
        void set_new_start_time(double new_start_time);

    protected:
        virtual void on_settings_changed  () {}
        virtual void on_start_time_changed() {}
    };

} // environment
