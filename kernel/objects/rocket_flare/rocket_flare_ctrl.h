#pragma once

#include "rocket_flare_view.h"

namespace rocket_flare
{

struct ctrl
    : view
    , control

{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    ctrl( kernel::object_create_t const& oc, dict_copt dict );

    // control
private:

    // base_presentation
private:
    void update( double /*time*/ ) override;


};



}