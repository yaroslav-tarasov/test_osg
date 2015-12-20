#pragma once

#include "flock_child_view.h"

namespace flock
{

namespace child
{

struct ctrl
    : view
    , control

{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    ctrl( kernel::object_create_t const& oc, dict_copt dict );

private:
    void set_model(const std::string&  icao_code) override;

};

}

}