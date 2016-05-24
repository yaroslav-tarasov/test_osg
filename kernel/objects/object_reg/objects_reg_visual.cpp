#include "stdafx.h"
#include "precompiled_objects.h"

#include "objects_reg_visual.h"

#include "objects/common/camera_common.h"

#include "common/ext_msgs.h"


namespace objects_reg
{

object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_visual, visual::create);

visual::visual( kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
	, _sys(oc.sys)
    , _vsys(dynamic_cast<visual_system*>(sys_))
{

    msg_disp()
        .add<net_layer::msg::traj_assign_msg       >(boost::bind(&visual::on_traj_assign , this, _1)) 
        ;

}

void visual::on_object_created(object_info_ptr object)
{

}

void visual::on_object_destroying(object_info_ptr object)
{    

}

void visual::on_traj_assign(net_layer::msg::traj_assign_msg const& msg) 
{
    if(msg.ext_id>0 )                          
    {
        traj_ = fms::trajectory::create(msg.traj);
        if(_vsys->scene()->GetTrajectoryDrawer())
            _vsys->scene()->GetTrajectoryDrawer()->set(traj_, cg::coloraf(1.0f,0.0f,0.0f,1.0f));

    }

}


} // end of objects_reg