#include "stdafx.h"
#include "precompiled_objects.h"
#include "camera_model.h"
#include "objects/nodes_management.h"

namespace camera_object
{

	object_info_ptr model::create(object_create_t const& oc, dict_copt dict)
	{
		Verify(dict);
		return object_info_ptr(new model(oc, dict));
	}

	model::model(object_create_t const& oc, dict_copt dict)
		: view          (oc, dict)
        , nm_ang_smooth_   (2)
	{
	}


    void  model::set_desired   (double time,const cg::point_3& pos, const cg::quaternion& orien, const double speed)
    {
        decart_position target_pos;

        target_pos.pos   = pos;
        target_pos.orien = orien;
        geo_position gtp(target_pos, get_base());

        if(!traj_)
        {
            traj_ = fms::trajectory::create();
            // LogInfo(" Re-create trajectory  " << settings_.custom_label );
        }

        traj_->append(time, pos, orien, speed);
		last_traj_update_ = last_update_;
    }


    void model::sync_nm_root(double dt)
    {
        Assert(root());

        if (!desired_nm_pos_ || !desired_nm_orien_)
            return;

        geo_point_3 const desired_pos   = *desired_nm_pos_;
        quaternion  const desired_orien = *desired_nm_orien_;

        nodes_management::node_position root_node_pos = root()->position();

        root_node_pos.global().dpos = geo_base_3(root_node_pos.global().pos)(desired_pos) / (dt/*sys_->calc_step()*/);
        root_node_pos.global().omega = cg::get_rotate_quaternion(root_node_pos.global().orien, desired_orien).rot_axis().omega() / (nm_ang_smooth_ * dt/*sys_->calc_step()*/);


        root()->set_position(root_node_pos);
    }

    void model::sync_traj( double time ) 
    {
        const double packet_delay = 1.0;

        if(traj_)
        {
            traj_->set_cur_len ((time-packet_delay>0)? time - packet_delay:0.0/*traj_->cur_len() + dt*/);
            const double  tar_len = traj_->cur_len();
            decart_position target_pos;

            target_pos.pos = cg::point_3(traj_->kp_value(tar_len));
            target_pos.orien = traj_->curs_value(tar_len);
            geo_position gtp(target_pos, get_base());


            desired_nm_pos_   = gtp.pos;
            desired_nm_orien_ = gtp.orien;
        }

    }


    void model::update( double time )
    {   
        view::update(time);

        double dt = time - (last_update_ ? *last_update_ : 0);
        if (cg::eq_zero(dt))
            return;

        desired_nm_pos_.reset();
        desired_nm_orien_.reset();

        sync_traj   (time);
        sync_nm_root(dt);

        last_update_ = time;
    }

}

AUTO_REG_NAME(camera_model, camera_object::model::create);