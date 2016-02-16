#pragma once

#include "human_common.h"
#include "human_msg.h"
#include "objects/nodes_management.h"
#include "common/human.h"


namespace human
{
    
//! сериализуемые данные из упражнения
struct human_data
{
    human_data()
    {
    }

    human_data(settings_t const& settings, state_t const& state)
        : settings_(settings)
        , state_   (state  )
    {
    }

protected:
    settings_t settings_;
    state_t    state_;

    REFL_INNER(human_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(state_)
    REFL_END()
};

//! вид
struct view
    : base_view_presentation            
    , obj_data_holder<human_data>     
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view( kernel::object_create_t const& oc, dict_copt dict);

    // base_view_presentation
protected:
    void on_child_removing(kernel::object_info_ptr child) override;



protected:
    void on_state   (state_t const& state);
    void on_settings(settings_t const& settings);
    void on_model_changed();

    void on_traj_assign         (msg::traj_assign_msg const &m);

public:
    geo_point_3     pos () const;
    cg::quaternion  orien() const {return state_.orien;}
	double          speed()  const {return state_.speed;}
    point_2         dpos() const {return point_2(cg::polar_point_2(1., state_.orien.cpr().course)) * state_.speed;}

	//info
public:

	std::string const& name() const;


protected:
    virtual void on_state_changed() {}    // Задействован только в чарте
    virtual void settings_changed() {}

public:
    void set_settings( settings_t const& settings );
    void set_state(state_t const&state);

protected:
    nodes_management::manager_ptr       nodes_manager_;
    nodes_management::node_control_ptr  root_;

protected:
    //////////////////////////////////////
    fms::trajectory_ptr                 traj_;
    /////////////////////////////////////

#if 1
private:
    using human_data::state_;
#endif
};

} // end of human
