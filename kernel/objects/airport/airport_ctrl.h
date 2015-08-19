#pragma once

#include "airport_view.h"

namespace airport
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
