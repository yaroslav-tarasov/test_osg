#pragma once

#include "aircraft_reg_view.h"


namespace aircraft_reg
{

struct model
    :  view
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    model(kernel::object_create_t const& oc, dict_copt dict);

private:
	bool add_aircraft(aircraft_physless::info_ptr airc_info);

private:
	void on_object_created(object_info_ptr object) override;
	void on_object_destroying(object_info_ptr object) override;

protected:
    void on_inject_msg(net_layer::msg::run const& msg);
    void on_inject_msg(net_layer::msg::malfunction_msg const& msg);
    void on_inject_msg(net_layer::msg::container_msg const& msg);
};

} // end of aircraft_reg