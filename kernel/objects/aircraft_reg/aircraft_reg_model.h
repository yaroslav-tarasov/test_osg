#pragma once

#include "aircraft_reg_view.h"


namespace aircraft_reg
{

struct model
    :  view
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    model(kernel::object_create_t const& oc, dict_copt dict);    

protected:
    void on_inject_msg(net_layer::test_msg::run const& msg);

};

} // end of aircraft_reg