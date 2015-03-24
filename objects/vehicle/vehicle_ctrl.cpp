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
}


