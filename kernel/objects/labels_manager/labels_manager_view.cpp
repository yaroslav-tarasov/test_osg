#include "precompiled_objects.h"

#include "labels_manager_view.h"

namespace labels_manager
{

    // fabric
    object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
    {
        return object_info_ptr(new view(oc, dict));
    }

    AUTO_REG_NAME(labels_manager_view, view::create);

    // ctor
    view::view( kernel::object_create_t const& oc, dict_copt dict )
        : base_view_presentation(oc)
    {
    }

}
