#include "objects/vehicle.h"
#include "objects/ada.h"
#include "vehicle_ctrl.h"



namespace vehicle
{

	object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
		return object_info_ptr(new ctrl(oc, dict));
	}

	AUTO_REG_NAME(vehicle_ext_ctrl, ctrl::create);

	ctrl::ctrl( kernel::object_create_t const& oc, dict_copt dict  )
		: view(oc,dict)
	{
        if (nodes_manager_)
        {
            if (nodes_manager_->get_model() != settings_.model)
                nodes_manager_->set_model(settings_.model);

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

	void ctrl::fire_fight()
	{
		aircraft::info_ptr burning_plane;
		bool reverse = false;

		visit_objects<aircraft::info_ptr>(collection_, [this, &burning_plane](aircraft::info_ptr air)->bool
		{       
			geo_point_3 tow_pos = geo_base_3(air->pos())(cg::rotation_3(cpr(air->orien().course, 0, 0)) * (point_3(0, 5., 0) ));
			if (cg::distance2d(tow_pos, this->pos()) < 25)
			{   
				burning_plane = air;
				return false;
			}

			return true;
		});

		if (burning_plane)
			send_cmd(msg::fight_fire_msg_t(object_info_ptr(burning_plane)->object_id(), 1));
	}

    void ctrl::attach_tow()
    {
        aircraft::info_ptr towair;
        bool reverse = false;

        visit_objects<aircraft::info_ptr>(collection_, [this, &towair,&reverse](aircraft::info_ptr air)->bool
        {       
            geo_point_3 tow_pos = geo_base_3(air->pos())(cg::rotation_3(cpr(air->orien().course, 0, 0)) * (point_3(0, 5., 0) + point_3(air->tow_point_transform().translation())));
            if (cg::distance2d(tow_pos, this->pos()) < 25)
            {   
                double dist_tp  = tow_point_node_ ?cg::distance2d(tow_pos,tow_point_node_->get_global_pos()) :10000.0;
                double dist_rtp = rtow_point_node_?cg::distance2d(tow_pos,rtow_point_node_->get_global_pos()):10000.0;

                if ( dist_rtp < dist_tp)
                {
                    reverse = true;
                }

                towair = air;
                return false;
            }
            
            return true;
        });

        if (towair)
            send_cmd(msg::attach_tow_msg_t(object_info_ptr(towair)->object_id(), reverse));
    }

    void ctrl::detach_tow()
    {
        send_cmd(msg::detach_tow_msg_t());
    }

    void ctrl::set_brake(double val)
    {
        send_cmd(msg::brake_msg_t(val));
    }

    void ctrl::set_reverse (bool val) 
    {
        send_cmd(msg::reverse_msg_t(val));
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

    void ctrl::on_aerotow_changed (aircraft::info_ptr old_aerotow, const boost::optional<msg::tow_msg> & msg)
    {
        view::on_aerotow_changed (old_aerotow, msg);
        geo_base_3 base = ::get_base();
        
        if(msg && !msg->tow_id)  
            detach_tow_signal_(decart_position(base((*msg).geo_pos.pos),(*msg).geo_pos.orien));
    }
}


