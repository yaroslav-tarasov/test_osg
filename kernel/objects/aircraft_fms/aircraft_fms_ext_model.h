#pragma once

#include "aircraft_fms_view.h"

#include "common/aircraft.h"

#include "fms/fms.h"
#include "fms/fms_tp.h"
//#include "common/fpl_manager.h"

namespace aircraft
{
namespace aircraft_fms
{

//! модель самолета
struct ext_model
    : model_presentation    
    , view                  
    , model_control
    , model_info
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

private:
    ext_model(kernel::object_create_t const& oc, dict_copt dict);

    // base_view_presentation
private:
    void on_object_created( object_info_ptr object ) override;

    // base_presentation
private:
    void update(double time);

    // model_control
private:
    void reset_pos(geo_point_3 const& pos, double c);
    void activate();

    // model_info
private:
    geo_point_3 prediction(double dt) const;

private:
    void on_state(msg::state_msg const& msg) override;
    void on_controls(fms::manual_controls_t const& ctrl) override;
    void on_transition(msg::transition_msg const& t);
    void on_fuel_discharge(double part);
    void on_plan_changed() override;

    void init_fms();
    
    void on_assigned_fpl_changed() override;
    void on_procedure_changed() override;
    void on_fms_settings_changed() override;

private:
    optional<double>    last_time_;

    fms::plan_traj_t        plan_traj_;
    fms::pilot_info_ptr     pilot_impl_;
    bool was_activated_;
};

}
}