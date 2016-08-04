#pragma once

#include "arresting_gear_view.h"

namespace arresting_gear
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
    void set_target(const boost::optional<uint32_t>&   target_id) override;

    // base_presentation
private:
    void update( double /*time*/ ) override;


};



}