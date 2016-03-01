#pragma once

#include "environment_view.h"
#include "kernel/kernel_fwd.h"

namespace environment
{
    struct ctrl
        : view
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        ctrl(kernel::object_create_t const& oc, dict_copt dict);

private:
    ctrl(kernel::object_create_t const& oc, istream_opt  stream);

private:
    void settings_edited    ();
    void ex_settings_changed();

private:
    scoped_connection ex_settings_changed_connection_ ;
    scoped_connection attr_changed_connection_ ;
};

} // environment
