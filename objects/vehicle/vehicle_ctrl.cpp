#include "stdafx.h"
#include "precompiled_objects.h"

#include "vehicle.h"
#include "vehicle_ctrl.h"
#include "objects/ada.h"


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

        //aircraft_fms::state_t st = get_fms_info()->get_state();
        //st.dyn_state.pos = p;
        //st.dyn_state.course = c;

        //if (ada::info_ptr ada_obj = find_first_object<ada::info_ptr>(collection_))
        //{
        //    if (auto adata = ada_obj->get_data(settings().kind))
        //    {
        //        fms::procedure_model_ptr proc_model = fms::create_bada_procedure_model(*adata) ;
        //        FIXME(Для наземки другой способ создания?) 
        //        // а то уедет ведь черт знает куда
        //        st.dyn_state.TAS = proc_model->taxi_TAS() ;// proc_model->nominal_cruise_TAS(p.height) ;
        //        st.dyn_state.fuel_mass = fms::calc_fuel_mass(settings().fuelload, *adata) ;
        //    }
        //}

        //if (cg::eq_zero(p.height))
        //    st.dyn_state.cfg = fms::CFG_GD;

        //aircraft_fms::control_ptr(get_fms_info())->set_state(st);
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
}


