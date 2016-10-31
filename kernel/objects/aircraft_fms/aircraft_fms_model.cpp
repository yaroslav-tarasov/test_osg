#include "aircraft_fms_model.h"

#include "fms/traj_calc.h"

//#include "common/fpl.h"

namespace aircraft
{
namespace aircraft_fms
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

AUTO_REG_NAME(aircraft_fms_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict )
    : view( oc, dict )
    , was_activated_(false)
{
    msg_disp()
        .add<msg::transition_msg>(boost::bind(&model::on_transition, this, _1))
        .add<msg::fuel_discharge_msg>(boost::bind(&model::on_fuel_discharge, this, _1))
        ;
}

void model::on_object_created( object_info_ptr object )
{
    bool had_fpl = get_fpl() ;

    view::on_object_created(object);

    if (!pilot_impl_ || (!had_fpl && get_fpl()))
        init_fms();
}

void model::update( double time )
{
    view::update(time);

    double const fms_calc_step = cfg().model_params.msys_step;
    if(pilot_impl_)
    {
        double dt = last_time_ ? time - *last_time_ : 0.;

        if (!cg::eq_zero(dt))
        {
            geo_base_3 prev_pos = pilot_impl_->state().dyn_state.pos;
            if (dt > 0)
            {
                size_t steps = cg::floor(dt / fms_calc_step + fms_calc_step * .1);
                fms::pilot_simulation_ptr pilot_sim(pilot_impl_);
                for (size_t i = 0; i < steps; ++i)
                {
                    pilot_sim->update(fms_calc_step);
                }
            }

            point_3 offset = prev_pos(pilot_impl_->state().dyn_state.pos);

            double course = pilot_impl_->state().dyn_state.course;
            double roll = pilot_impl_->roll();

            cpr orien(course, polar_point_3(offset).pitch, roll);
            
            set_state(state_t(pilot_impl_->state(), orien.pitch, orien.roll, state_.version + 1), false);

            if (pilot_impl_->instrument() && pilot_impl_->instrument_ended())
            {
                fms::plan_t plan;
                if (get_plan().size() > 1)
                {
                    auto tmp_plan = fms::plan_t(get_plan().begin()+1, get_plan().end());
                    std::swap(plan, tmp_plan);
                }
                else if (pilot_impl_->instrument()->kind() == fms::INSTRUMENT_APPROACH && pilot_impl_->state().dyn_state.cfg != fms::CFG_GD)
                {
                    plan.push_back(fms::create_direction_instrument(boost::none, get_instruments_env())) ;
                }

                set_plan(plan);
            }
        }
    }

    last_time_ = time;
}

void model::reset_pos(geo_point_3 const& pos, double c)
{
    if (pilot_impl_)
    {
        fms::pilot_state_t state =  pilot_impl_->state();
        state.dyn_state.pos = pos;
        state.dyn_state.course = c;

        fms::pilot_simulation_ptr(pilot_impl_)->reset_state(state);
    }
}

void model::activate()
{
    if (!get_instrument() && !was_activated_)
    {
        ani::navigation_ptr navi = ani_ ? ani_->navigation_info() : ani::navigation_ptr();
        fms::plan_t plan =  fms::create_instruments_plan(get_state(), get_instruments_env()) ;

        set_plan(plan);
        was_activated_ = true;
    }
}

geo_point_3 model::prediction(double dt) const
{
    if (pilot_impl_)
        return pilot_impl_->prediction(dt);

    return get_state().dyn_state.pos;
}

void model::on_state(msg::state_msg const& msg)
{
    fms::pilot_state_t fmsst = (state_t)msg;

    if (pilot_impl_)
        fms::pilot_simulation_ptr(pilot_impl_)->reset_state(fmsst);

    view::on_state(msg);
}

void model::on_controls(fms::manual_controls_t const& ctrl)
{
    if (pilot_impl_)
        fms::pilot_control_ptr(pilot_impl_)->reset_controls(ctrl);

    view::on_controls(ctrl);
}

void model::on_transition(msg::transition_msg const& t)
{
    set_auto_transition((fms::transition_t)*t, true);

//     if (pilot_impl_)
//         fms::pilot_control_ptr(pilot_impl_)->set_transition((fms::transition_t)*t, true);
}

void model::on_fuel_discharge(double part)
{
    if (pilot_impl_)
    {
        double max_fuel_mass = max_fuel();
        fms::pilot_control_ptr(pilot_impl_)->fuel_discharge(part * max_fuel_mass);
    }
}

void model::on_plan_changed()
{
    view::on_plan_changed();

    fms::pilot_control_ptr(pilot_impl_)->set_instrument(!get_plan().empty() ? get_plan().front() : fms::instrument_ptr());
}

void model::init_fms()
{
    if (!fsettings() || !get_meteo_proxy())
        return ;

    fms::pilot_state_t fmsst = get_state();

    ada::data_t const& fsetttings = *fsettings();
    fms::aircraft_data_t adata(fsetttings, payload_ * fsetttings.max_payload_mass);

    pilot_impl_ = fms::create_aircraft_pilot(get_meteo_proxy(), fmsst, adata);      

    fms::pilot_control_ptr(pilot_impl_)->set_instrument(!get_plan().empty() ? get_plan().front() : fms::instrument_ptr());
}

void model::on_assigned_fpl_changed()
{
    view::on_assigned_fpl_changed();
    init_fms();
}

void model::on_procedure_changed()
{
    view::on_procedure_changed();
    init_fms();
}

void model::on_fms_settings_changed()
{
    view::on_fms_settings_changed();
    init_fms();
}

}
}
