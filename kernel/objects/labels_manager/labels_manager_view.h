#pragma once

namespace labels_manager
{

    struct view
        : base_view_presentation
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    protected:
        view( kernel::object_create_t const& oc, dict_copt dict );
    };

}
