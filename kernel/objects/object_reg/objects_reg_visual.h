#pragma once

#include "objects_reg_view.h"

namespace objects_reg
{

struct visual
    : view   
{

    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    visual(kernel::object_create_t const& oc, dict_copt dict);    

private:
    void on_object_created   (object_info_ptr object) override;
    void on_object_destroying(object_info_ptr object) override;

    // info
private:
    // 
private:
    void on_traj_assign(net_layer::msg::traj_assign_msg const& msg);

protected:
	kernel::system*                                                     _sys;
    kernel::visual_system*                                             _vsys;
private:
    connection_holder                                           conn_holder_;

    fms::trajectory_ptr                                                traj_;
};

} // end of objects_reg