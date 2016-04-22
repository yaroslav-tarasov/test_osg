#include "stdafx.h"
#include "precompiled_objects.h"

#include "mdd_view.h"

namespace mdd
{
    object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
    {
        return object_info_ptr(new view(oc, dict));
    }


    view::view( kernel::object_create_t const& oc, dict_copt)
        : base_view_presentation(oc)
    {

    }

    void view::on_msg( short /*id*/, const void * /*data*/, size_t /*size*/ )
    {

    }

    AUTO_REG_NAME(mdd_view , view::create);
}
