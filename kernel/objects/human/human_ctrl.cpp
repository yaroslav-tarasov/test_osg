#include "stdafx.h"
#include "precompiled_objects.h"

#include "human.h"
#include "human_ctrl.h"
#include "objects/ada.h"


namespace human
{

	object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new ctrl(oc, dict));
	}

	AUTO_REG_NAME(human_ext_ctrl, ctrl::create);

	ctrl::ctrl( kernel::object_create_t const& oc, dict_copt dict  )
		: view(oc,dict)
	{
        ctrl_system* vsys = dynamic_cast<ctrl_system*>(sys_);

        if (nodes_manager_)
        {
            if (nodes_manager_->get_model() != settings_.model)
                nodes_manager_->set_model(settings_.model);

            //if (neo)
                root_->set_position(geo_position(geo_point_3(pos(), 0), point_3(), quaternion(cpr(course(), 0, 0)), point_3()));
        }

	}

    void ctrl::update(double time)
    {
        view::update(time);
        //update_len(time);

    }

    void ctrl::set_initial_position( cg::geo_point_3 const &p, double c)
    {
        nodes_management::node_control_ptr root(/*get_nodes_manager()*/nodes_manager_->get_node(0));
        root->set_position(geo_position(p, quaternion(cpr(c, 0, 0))));
        FIXME(А у самолета тут гораздо больше кода)
    }

    void ctrl::goto_pos(geo_point_2 pos,double course)
    {
        send_cmd(msg::go_to_pos_data(pos,course));
    }

    void ctrl::follow_route(std::string const& route)
    {
        object_info_ptr routeptr = find_object<object_info_ptr>(collection_, route);
        if (routeptr)
        {
            send_cmd(msg::follow_route_msg_t(routeptr->object_id()));
        }
    }

    void ctrl::attach_tow()
    {
        aircraft::info_ptr towair;

        visit_objects<aircraft::info_ptr>(collection_, [this, &towair](aircraft::info_ptr air)->bool
        {       
            geo_point_3 tow_pos = geo_base_3(air->pos())(cg::rotation_3(cpr(air->orien().course, 0, 0)) * (point_3(0, 5., 0) + point_3(air->tow_point_transform().translation())));
            if (cg::distance2d(tow_pos, this->pos()) < 25)
            {              
                towair = air;
                return false;
            }

            return true;
        });

        if (towair)
            send_cmd(msg::attach_tow_msg_t(object_info_ptr(towair)->object_id()));
    }

    void ctrl::detach_tow()
    {
        send_cmd(msg::detach_tow_msg_t());
    }

    void ctrl::set_brake(double val)
    {
        send_cmd(msg::brake_msg_t(val));
    }

    void ctrl::follow_trajectory(std::string const& /*route*/)
    {
        send_cmd(msg::follow_trajectory_msg_t(0));
    }

    void ctrl::set_trajectory(fms::trajectory_ptr  traj)
    {
        traj_ = traj;
        fms::traj_data data(*traj);
        set(msg::traj_assign_msg(data));
    }

    fms::trajectory_ptr  ctrl::get_trajectory()
    {
        return traj_;
    } 

    decart_position ctrl::get_local_position() const
    {
        FIXME(Oooooooooooooo)
        geo_base_3 base = ::get_base();

        return decart_position(base(geo_point_3(pos(),0)),cg::cpr(course(),0,0));
    };
}


