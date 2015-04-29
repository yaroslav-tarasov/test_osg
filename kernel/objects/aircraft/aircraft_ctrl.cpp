#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_ctrl.h"
#include "objects/ada.h"

namespace aircraft
{

	object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
	{   
        cg::geo_point_3 b_pos(0.000,0.000,0.000);
        if (dict)
        {
            return object_info_ptr(new ctrl(oc, *dict));
        }
        else
    		return object_info_ptr(new ctrl(oc, dict,b_pos,double(0)));
	}

	AUTO_REG_NAME(aircraft_ext_ctrl, ctrl::create);

	ctrl::ctrl( kernel::object_create_t const& oc, dict_copt dict , optional<cg::geo_point_3> const &initial_pos , optional<double> const &initial_course )
		: view(oc,dict)
	{
        ctrl_system* vsys = dynamic_cast<ctrl_system*>(sys_);
        
        if(initial_pos && initial_course)
            set_initial_position(*initial_pos,*initial_course);

        FIXME(Или или)
        
        if(dict)
            set_initial_position(state_.pos,state_.orien.get_course());
	}

    void ctrl::update(double time)
    {
        view::update(time);
        update_len(time);

    }

    void ctrl::set_initial_position( cg::geo_point_3 const &p, double c)
    {
        nodes_management::node_control_ptr root(get_nodes_manager()->get_node(0));
        root->set_position(geo_position(p, quaternion(cpr(c, 0, 0))));

        aircraft_fms::state_t st = get_fms_info()->get_state();
        st.dyn_state.pos = p;
        st.dyn_state.course = c;

        if (ada::info_ptr ada_obj = find_first_object<ada::info_ptr>(collection_))
        {
            if (auto adata = ada_obj->get_data(settings().kind))
            {
                fms::procedure_model_ptr proc_model = fms::create_bada_procedure_model(*adata) ;
                FIXME(Для наземки другой способ создания?) 
                // а то уедет ведь черт знает куда
                st.dyn_state.TAS = proc_model->taxi_TAS() ;// proc_model->nominal_cruise_TAS(p.height) ;
                st.dyn_state.fuel_mass = fms::calc_fuel_mass(settings().fuelload, *adata) ;
            }
        }

        if (cg::eq_zero(p.height))
            st.dyn_state.cfg = fms::CFG_GD;

        aircraft_fms::control_ptr(get_fms_info())->set_state(st);
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

    decart_position ctrl::get_local_position()
    {
        FIXME(Oooooooooooooo)
        geo_base_3 base = ::get_base();
        //aircraft_fms::state_t st = get_fms_info()->get_state();
        return decart_position(base(pos()),orien()/*cg::cpr(st.dyn_state.course,0,0)*/);
    };



}


