#pragma once

#include "common/debug_render.h"
#include "mdd_msgs.h"

namespace mdd
{
    struct view
        : base_view_presentation
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    protected:
        view( kernel::object_create_t const& oc, dict_copt dict );

        // base_presentation
    private:
        void on_msg( short id, const void * data, size_t size );
    };
}