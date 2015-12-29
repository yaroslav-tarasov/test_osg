#pragma once

#include "objects_reg_view.h"


namespace objects_reg
{

struct model
    :  view
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    model(kernel::object_create_t const& oc, dict_copt dict);

private:
	bool add_object(object_info_ptr object);

private:
	void on_object_created(object_info_ptr object) override;
	void on_object_destroying(object_info_ptr object) override;

protected:
    void on_inject_msg(net_layer::msg::run const& msg);
    void on_inject_msg(net_layer::msg::malfunction_msg const& msg);
    void on_inject_msg(net_layer::msg::container_msg const& msg);

private:
    std::unordered_map<object_id_t, net_layer::msg::run>  last_msg_;
};

} // end of objects_reg