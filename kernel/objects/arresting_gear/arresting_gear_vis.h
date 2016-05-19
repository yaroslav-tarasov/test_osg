#pragma once

#include "arresting_gear_view.h"

namespace arresting_gear
{

struct visual
    : view

{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    visual( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
private:
    void update( double /*time*/ ) override;


};



}