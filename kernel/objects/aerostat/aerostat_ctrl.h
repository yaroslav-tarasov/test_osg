#pragma once

#include "aerostat_view.h"

namespace aerostat
{

struct ctrl
    : view
    , control

{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    ctrl( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
private:
    void update( double /*time*/ ) override;

    bool                           anim_started;

};



}