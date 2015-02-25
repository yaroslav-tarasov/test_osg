#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_fms_view.h"
#include "objects/fpl.h"
#include "fms/traj_calc.h"
#include "fms/fms_instruments.h"
#include "fms/fms_holding.h"

namespace aircraft
{
namespace aircraft_fms
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

view::view( kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base(dict)
    , ada_         (find_first_object<ada::info_ptr>(collection_))
    , ani_         (find_first_object<ani_object::info_ptr>(collection_))
    //, meteo_proxy_ (find_first_object<meteo_proxy::info_ptr>(collection_)->get_general_proxy())
    //, meteo_cursor_(meteo_proxy_->create_cursor())
{
    //LogDebug("aircraft_fms::view created");
    instruments_env_.navi = ani_->navigation_info();

    msg_disp()
        .add<msg::state_msg>   (boost::bind(&view::on_state, this, _1))
        .add<msg::kind_msg>    (boost::bind(&view::on_kind, this, _1))
        .add<msg::payload_msg> (boost::bind(&view::on_payload, this, _1))
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1))
        .add<msg::controls_msg>(boost::bind(&view::on_controls, this, _1))
        .add<msg::plan_msg>    (boost::bind(&view::on_plan, this, _1))
        ;
}

void view::update(double time)
{
    base_view_presentation::update(time);
    closest_airport_ = ani_->navigation_info()->find_airport(state_.dyn_state.pos, 70000);
}

void view::on_object_created( object_info_ptr object )
{
    base_view_presentation::on_object_created(object);

    if (fpl::info_ptr f = object)
    {
        aircraft::info_ptr aircrft = parent().lock();
        if (object == aircrft->get_fpl())
        {
            Assert(!fpl_) ;
            fpl_ = f;

            if (auto traj = fpl_->traj())
            {
                procedure_ = traj->procedure(fms::PROC_ROUTE);
                instruments_env_.procedure = procedure_;
                instruments_env_.taxi_procedure = traj->procedure(fms::PROC_TAXI_1);
                instruments_env_.traj_data = traj->traj_data();
            }

            procedure_changed_conn_ = fpl_->subscribe_procedure_changed(boost::bind(&view::on_procedure_changed, this));
        }
    }
}

void view::on_object_destroying( object_info_ptr object ) 
{
    base_view_presentation::on_object_destroying(object);

    if (fpl_ == object)
    {
        procedure_changed_conn_ = connection();
        procedure_.reset();
        instruments_env_.procedure.reset();
        instruments_env_.taxi_procedure.reset();
        fpl_.reset();
    }
}

void view::on_parent_changed()
{
    base_view_presentation::on_parent_changed();

    aircraft::info_ptr aircrft = parent().lock();
    assigned_fpl_changed_conn_ = connection();

    if (aircrft)
    {
        // TODO
        aircraft_kind_ = aircrft->settings().kind;
        if (!aircrft->settings().parking.empty())
        {
            if (auto pnt = ani_->navigation_info()->find_point(aircrft->settings().parking))
                instruments_env_.traj_data.attr.depart_parking = pnt->second;
        }

        make_aircraft_info();


        assigned_fpl_changed_conn_ = aircrft->subscribe_assigned_fpl_changed(boost::bind(&view::on_assigned_fpl_changed, this)) ;
        on_assigned_fpl_changed();
    }
}

state_t const& view::get_state()  const
{
    return state_;
}

double view::get_mass() const
{
    return operation_model()->calc_mass(state_.dyn_state.fuel_mass, get_payload_mass());
}

double view::get_payload_mass() const
{
    return payload_ * aircraft_data_->max_payload_mass;
}

fms::manual_controls_t const& view::get_controls()  const
{
    return controls_;
}

fms::instrument_ptr  view::get_instrument() const 
{
    return !plan_.empty() ? plan_.front() : fms::instrument_ptr();
}

fms::instrument_ptr  view::get_next_instrument() const
{
    return plan_.size() > 1 ? plan_[1] : fms::instrument_ptr();
}

double view::length_fwd() const
{
    return aircraft_data_ ? 5 + aircraft_data_->length / 2 : 0;
}

double view::length_bwd() const
{
    return aircraft_data_ ? aircraft_data_->length / 2 : 0;
}

optional<ada::data_t> const& view::fsettings() const
{
    return aircraft_data_;
}

fms::procedure_model_ptr view::procedure_model() const
{
    return procedure_model_;
}

fms::operation_model_ptr view::operation_model() const
{
    return procedure_model_;
}

point_3 view::ground_velocity()  const
{
    point_3 wind;
    // FIXME
    //if (meteo_cursor_)
    //{
    //    meteo_cursor_->move_to(state_.dyn_state.pos, 0);
    //    wind = meteo_cursor_->wind();
    //}

    point_3 vel = cg::polar_point_3(state_.dyn_state.TAS, state_.orien().course, state_.orien().pitch);

    return state_.dyn_state.cfg == fms::CFG_GD ? vel : vel + wind;
}

double view::max_fuel() const
{
    if (aircraft_data_)
    {
        return aircraft_data_->max_mass - aircraft_data_->min_mass - aircraft_data_->max_payload_mass;
    }

    return 1;
}

ani::airport_info_ptr view::closest_airport() const 
{
    return closest_airport_;
}

optional<double> view::get_desired_course() const
{
    if (!plan_.empty())
    {
        fms::instrument_ptr instr = plan_.front();
        if (instr->kind() == fms::INSTRUMENT_DIRECTION)
            return fms::instrument_direction_ptr(instr)->course();
        else if (instr->kind() == fms::INSTRUMENT_DIRECTION_GROUND)
            return fms::instrument_direction_ptr(instr)->course();
        else if (instr->kind() == fms::INSTRUMENT_TURN)
            return fms::instrument_turn_ptr(instr)->course();
    }
    return boost::none;
}

optional<double> view::get_desired_height() const
{
    if (!plan_.empty())
    {
        auto instr = fms::instrument_desired_info_ptr(plan_.front());
        if (instr)
            return instr->desired_height();
    }
    return boost::none;
}

optional<double> view::get_desired_CAS() const
{
    if (!plan_.empty())
    {
        auto instr = fms::instrument_desired_info_ptr(plan_.front());
        if (instr)
            return instr->desired_CAS();
    }
    return boost::none;
}

optional<double> view::get_desired_ROCD() const
{
    if (!plan_.empty())
    {
        auto instr = fms::instrument_desired_info_ptr(plan_.front());
        if (instr)
            return instr->desired_ROCD();
    }
    return boost::none;
}

void view::set_state(state_t const& state)
{
    set_state(state, true);
}

void view::set_controls( fms::manual_controls_t const& ctrl )
{
    set(msg::controls_msg(ctrl), true);
}

void view::set_plan(fms::plan_t const& plan)
{
    if (!plan.empty())
        plan.front()->activate(state_, get_instrument());

    std::vector<binary::bytes_t> plan_data;

    for(size_t i = 0; i < plan.size(); ++i)
        plan_data.push_back(fms::save_instrument(plan[i]));

    set(msg::plan_msg(plan_data), true);
}


void view::transition(fms::transition_t t)
{
    send_cmd(msg::transition_msg(t));
}

void view::fuel_discharge(double part) 
{
    send_cmd(msg::fuel_discharge_msg(part));
}

void view::set_aircraft_kind(std::string const &kind)
{
    if (aircraft_kind_ != kind)
        set(msg::kind_msg(kind));
}

void view::set_payload(double payload)
{
    if (payload_ != payload)
        set(msg::payload_msg(payload));
}

void view::instrument_direction(double course)
{
    ani::navigation_ptr navi = ani_ ? ani_->navigation_info() : ani::navigation_ptr();

    fms::instrument_ptr instrument = state_.dyn_state.cfg != fms::CFG_GD ? 
                                            fms::create_direction_instrument(course, instruments_env_) :
                                            fms::create_direction_ground_instrument(course, instruments_env_);

    fms::plan_t plan;
    plan.push_back(instrument);
    set_plan(plan);
}

void view::instrument_turn(double course, optional<bool> ccw, optional<double> roll)
{
    Assert(state_.dyn_state.cfg != fms::CFG_GD);

    ani::navigation_ptr navi = ani_ ? ani_->navigation_info() : ani::navigation_ptr();

    fms::plan_t plan;

    fms::instrument_ptr instrument = fms::create_turn_instrument(course, ccw, roll, instruments_env_);
    plan.push_back(instrument);

    fms::instrument_ptr direction_instrument = fms::create_direction_instrument(course, instruments_env_);
    plan.push_back(direction_instrument);

    set_plan(plan);
}

bool view::instrument_point(ani::point_pos const& p)
{
    if (state_.dyn_state.cfg == fms::CFG_GD)
        return false;

    bool point_found = false;
    bool procedure_return = false;

    fpl::info_ptr fpl_info = aircraft::info_ptr(parent().lock())->get_fpl();
    if (fpl_info)
    {
        if (fpl_info->contains_point(p))
        {
            point_found = true;
            procedure_return = true;
        }
    }
    
    if (!point_found && ani_)
    {
        ani::navigation_ptr navi = ani_->navigation_info();
        auto pnt_id = navi->find_point(ani::air_point(p));
        if (pnt_id)
        {
            point_found = true;
        }
    }

    if (point_found)
    {

        fms::plan_t plan;

        optional<double> next_course;
        if (procedure_return)
        {
            double len = fpl_info->traj()->plan_traj().route.closest(p);
            next_course = fpl_info->traj()->plan_traj().route.course(len + 0.1);
        }                                                               

        fms::instrument_ptr instrument = fms::create_point_instrument(p, next_course, instruments_env_);
        plan.push_back(instrument);

        if (procedure_return)
        {
            fms::plan_t initial_plan_clon  = std::move(fms::clone_plan(fpl_info->traj()->get_initial_plan()));

            size_t i = 0;
            for (;i < initial_plan_clon.size(); ++i)
                if (initial_plan_clon[i]->kind() == fms::INSTRUMENT_FPL)
                    break;
            for (;i < initial_plan_clon.size(); ++i)
                plan.push_back(initial_plan_clon[i]);
        }
        else
        {
            fms::instrument_ptr dir_instrument = fms::create_direction_instrument(boost::none, instruments_env_);
            plan.push_back(dir_instrument) ;
        }

        set_plan(plan);

        return true;
    }

    return false;
}

void view::instrument_lock     (string const& airport, ani::runway_id runway)
{
    if (ani_)
    {
        ani::navigation_ptr navi = ani_->navigation_info();

        auto a = navi->get_airport(airport);

        geo_point_2 pos = navi->get_point_info(ani::ground_point_id(runway.first)).pos;
        double course = cg::norm180(a->get_runway_info(runway)->course() + 180.);

        fms::plan_t plan;

        fms::instrument_ptr instrument = get_instrument();
        if (!fms::instrument_approach_ptr(instrument))
        {
            fms::condition_ptr cond = fms::create_capture_course_condition(pos, course);
            instrument->add_end_condition(cond);
            plan.push_back(instrument);
        }

        fms::instrument_ptr approach_instrument = fms::create_lock_instrument(runway, instruments_env_);

        plan.push_back(approach_instrument);
        set_plan(plan);
    }
}

void view::instrument_approach (string const& airport, ani::runway_id runway)
{
    if (ani_)
    {
        ani::navigation_ptr navi = ani_->navigation_info();

        auto a = navi->get_airport(airport);

        geo_point_2 pos = navi->get_point_info(ani::ground_point_id(runway.first)).pos;
        double course = cg::norm180(a->get_runway_info(runway)->course() + 180.);

        fms::plan_t plan;

        fms::instrument_ptr instrument = get_instrument();
        if (!fms::instrument_approach_ptr(instrument))
        {
            fms::condition_ptr cond = fms::create_capture_course_condition(pos, course);
            instrument->add_end_condition(cond);
            plan.push_back(instrument);
        }

        fms::instrument_ptr approach_instrument = fms::create_approach_instrument(runway, instruments_env_);
        plan.push_back(approach_instrument);
        set_plan(plan);
    }
}

void view::instrument_go_around (string const& airport, ani::runway_id runway)
{
    if (ani_)
    {
        ani::navigation_ptr navi = ani_->navigation_info();

        fms::plan_t plan;

        optional<ani::sidstar_id> go_around_id = navi->find_around_route(airport, runway);
        if (go_around_id)
        {
            fms::instrument_ptr go_around_instrument = fms::create_go_around_instrument(*go_around_id, instruments_env_);
            plan.push_back(go_around_instrument);

            fms::instrument_ptr approach_instrument = fms::create_approach_instrument(runway, instruments_env_);

            plan.push_back(approach_instrument);
            set_plan(plan);
        }
    }
}

void view::instrument_star(ani::sidstar_id id)
{
    fms::instrument_ptr instrument = get_instrument();
    if (!instrument)
        return;

    if (instrument->kind() == fms::INSTRUMENT_POINT)
    {
        fms::plan_t plan;
        plan.push_back(instrument);
        fms::instrument_ptr star_instrument = fms::create_star_instrument(id, instruments_env_);
        plan.push_back(star_instrument);

        // approach
        ani::navigation_ptr navi = ani_->navigation_info();
        ani::airport_info_ptr a = navi->get_airport(navi->get_procedure_sidstar(id).airport);
        auto runway = a->find_runway(navi->get_procedure_sidstar(id).runway);
        Assert(runway);
        fms::instrument_ptr approach_instrument = fms::create_approach_instrument(*runway, instruments_env_);
        plan.push_back(approach_instrument);

        set_plan(plan);
    }
    else
    {
        fms::instrument_fpl_ptr plan_fpl_instr = fms::instruments::find_first_instrument<fms::INSTRUMENT_FPL>(get_plan());
        if (plan_fpl_instr)
        {
            fms::plan_t plan;
            size_t i = 0;
            for(;i < get_plan().size() && get_plan()[i] != plan_fpl_instr; ++i)
                plan.push_back(get_plan()[i]);
            plan.push_back(plan_fpl_instr);
            ++i;
            
            fms::instrument_ptr star_instrument = fms::create_star_instrument(id, instruments_env_);
            plan.push_back(star_instrument);

            // approach
            ani::navigation_ptr navi = ani_->navigation_info();
            ani::airport_info_ptr a = navi->get_airport(navi->get_procedure_sidstar(id).airport);
            auto runway = a->find_runway(navi->get_procedure_sidstar(id).runway);
            Assert(runway);
            fms::instrument_ptr approach_instrument = fms::create_approach_instrument(*runway, instruments_env_);
            plan.push_back(approach_instrument);


            set_plan(plan);
        }
    }
}

void view::instrument_holding  (ani::point_id id, fms::holding::config_t const &cfg)
{
    fms::instrument_ptr instrument = get_instrument();
    if (!instrument)
        return;

    ani::navigation_ptr navi = ani_->navigation_info();

    fms::holding::config_t final_cfg = cfg;
    if (instrument->kind() == fms::INSTRUMENT_POINT)
    {
        fms::plan_t plan;
        plan.push_back(instrument);

        final_cfg.pos = navi->get_point_info(id).pos;
        
        fms::instrument_ptr holding_instrument = fms::create_holding_instrument(final_cfg, instruments_env_);
        plan.push_back(holding_instrument);

        fms::instrument_ptr dir_instrument = fms::create_direction_instrument(boost::none, instruments_env_);
        plan.push_back(dir_instrument) ;

        set_plan(plan);
    }
    else if (instrument->kind() == fms::INSTRUMENT_FPL)
    {
        fms::plan_t plan;

        fms::instrument_ptr fpl_clon = instrument->clone();

        fpl_clon->add_end_condition(fms::create_pass_fpl_point_condition(navi->get_point_info(id).pos));
        plan.push_back(fpl_clon);
        final_cfg.pos = navi->get_point_info(id).pos;
        fms::instrument_ptr holding_instrument = fms::create_holding_instrument(final_cfg, instruments_env_);
        plan.push_back(holding_instrument);

        for (auto it = get_plan().begin(); it != get_plan().end(); ++it)
            plan.push_back(*it);

        set_plan(plan);
    }
}

void view::instrument_cancel_next()
{
    fms::instrument_ptr instrument = get_instrument();
    fms::instrument_ptr next_instrument = get_next_instrument();
    if (!instrument || !next_instrument)
        return;

    fms::plan_t plan;

    if (next_instrument->kind() == fms::INSTRUMENT_HOLDING && instrument->kind() == fms::INSTRUMENT_FPL)
    {
        Assert(get_plan().size() > 2);
        plan = fms::plan_t(get_plan().begin()+2, get_plan().end());
        Assert(plan.front()->kind() == fms::INSTRUMENT_FPL);
    }
    else if (next_instrument->kind() == fms::INSTRUMENT_STAR || 
             next_instrument->kind() == fms::INSTRUMENT_APPROACH ||
             next_instrument->kind() == fms::INSTRUMENT_SID)
    {
        plan.push_back(instrument);
        plan.push_back(fms::create_direction_instrument(boost::none, instruments_env_));
    }
    else if (next_instrument->kind() == fms::INSTRUMENT_FPL && instrument->kind() == fms::INSTRUMENT_FPL)
    {
        plan.push_back(instrument);
        fms::instrument_fpl_ptr(instrument)->set_end_point(boost::none);
        auto it = get_plan().begin();
        for (; it != get_plan().end(); ++it)
            if ((*it)->kind()!=fms::INSTRUMENT_FPL)
                break;
        for (; it != get_plan().end(); ++it)
            plan.push_back(*it);
    }
    else
    {
        plan.push_back(instrument);

        if (get_plan().size() > 2)
        {
            plan.insert(plan.end(), get_plan().begin()+2, get_plan().end());
        }
    }

    if (plan.empty() || (plan.back()->kind() != fms::INSTRUMENT_FPL && plan.back()->kind() != fms::INSTRUMENT_DIRECTION && plan.back()->kind() != fms::INSTRUMENT_APPROACH))
    {
        plan.push_back(fms::create_direction_instrument(boost::none, instruments_env_));
    }

    set_plan(plan);
}

void view::instrument_fpl_offset(double value, optional<geo_point_2> start, optional<geo_point_2> end)
{
    fms::plan_t new_plan;

    if (!get_plan().empty())
    {
        fms::instrument_ptr instr = get_plan().front();
        if (instr->kind() == fms::INSTRUMENT_FPL)
        {
            if (cg::eq_zero(value))
            {
                new_plan.push_back(instr);
                fms::instrument_fpl_ptr(instr)->set_offset(0.);
                fms::instrument_fpl_ptr(instr)->set_end_point(boost::none);
            }
            else
            {
                if (start)
                {
                    fms::instrument_fpl_ptr start_instr = instr->clone();
                    start_instr->set_end_point(*start);
                    start_instr->set_offset(0);
                    new_plan.push_back(start_instr);
                }

                fms::instrument_fpl_ptr(instr)->set_offset(value);
                new_plan.push_back(instr);

                if (end)
                {
                    fms::instrument_fpl_ptr end_instr = instr->clone();
                    end_instr->set_offset(0);
                    end_instr->set_end_point(boost::none);
                    new_plan.push_back(end_instr);

                    fms::instrument_fpl_ptr(instr)->set_end_point(*end);
                }
            }

            auto it = get_plan().begin();
            for(; it != get_plan().end(); ++it)
                if (!fms::instrument_fpl_ptr(*it))
                    break;
            for(; it != get_plan().end(); ++it)
                new_plan.push_back(*it);

            set_plan(new_plan);
        }
    }
}

void view::instrument_point_fly_over(bool fly_over)
{
    fms::plan_t new_plan = get_plan() ;

    if (!new_plan.empty())
    {
        if (fms::instrument_point_ptr point_instr = new_plan.front())
        {
            point_instr->set_fly_over(fly_over);
            set_plan(new_plan);
        }
    }
}

void view::instrument_point_height_mode(fms::instruments::point::height_mode_t hm)
{
    fms::plan_t new_plan = get_plan() ;

    if (!new_plan.empty())
    {
        if (fms::instrument_point_ptr point_instr = new_plan.front())
        {
            point_instr->set_height_mode(hm);
            set_plan(new_plan);
        }
    }
}


void view::set_desired_height (optional<double> height)
{
    fms::plan_t new_plan = get_plan() ;

    if (!new_plan.empty())
    {
        if (fms::instrument_base_control_ptr instr_control = new_plan.front())
        {
            instr_control->set_height(height);
            if (!height)
                instr_control->set_ROCD(boost::none);
            set_plan(new_plan);
        }
    }
}

void view::set_desired_CAS    (optional<double> CAS)
{
    fms::plan_t new_plan = get_plan() ;

    if (!new_plan.empty())
    {
        if (fms::instrument_base_control_ptr instr_control = new_plan.front())
        {
            instr_control->set_CAS(CAS);
            set_plan(new_plan);
        }
    }
}

void view::set_desired_ROCD(optional<double> ROCD)
{
    fms::plan_t new_plan = get_plan() ;

    if (!new_plan.empty())
    {
        if (fms::instrument_base_control_ptr instr_control = new_plan.front())
        {
            instr_control->set_ROCD(ROCD);
            set_plan(new_plan);
        }
    }
}

void view::set_state(state_t const& st, bool sure)
{                   
    set(msg::state_msg(st), sure);
}

void view::set_auto_transition(fms::transition_t i, bool isauto)
{
    auto new_settings = settings_;
    new_settings.auto_transition[i] = isauto;
    set(msg::settings_msg(new_settings));
}

#if 0
meteo::meteo_proxy_ptr view::get_meteo_proxy() const
{
    return meteo_proxy_;
}
#endif

void view::on_fms_settings_changed()
{
    if (fpl_)
    {
        procedure_ = fpl_->traj()->procedure(fms::PROC_ROUTE);
        instruments_env_.procedure = procedure_;
        instruments_env_.taxi_procedure = fpl_->traj()->procedure(fms::PROC_TAXI_1);
        instruments_env_.traj_data = fpl_->traj()->traj_data();
    }
    fms_changed_signal_();
}

void view::on_plan_changed()
{
}

void view::on_state(msg::state_msg const& msg)
{
    state_ = msg;
    state_changed_signal_();
}

void view::on_controls(fms::manual_controls_t const& ctrl)
{
    controls_ = ctrl;
}

void view::on_plan(std::vector<binary::bytes_t> const& data)
{
    ani::navigation_ptr navi = ani_ ? ani_->navigation_info() : ani::navigation_ptr();

    plan_.clear();
    for (size_t i = 0; i < data.size(); ++i)
        plan_.push_back(fms::load_instrument(data[i], instruments_env_));
    on_plan_changed();
    plan_changed_signal_();
}

void view::on_kind(string const& kind)
{
    aircraft_kind_ = kind;

    make_aircraft_info();
    on_fms_settings_changed();
}

void view::on_payload(double payload)
{
    payload_ = payload;
}

void view::on_settings(settings_t const& settings)
{
    settings_ = settings;
}

void view::on_procedure_changed()
{
    Assert(fpl_);
    procedure_ = fpl_->traj()->procedure(fms::PROC_ROUTE);
    instruments_env_.procedure = procedure_;
    instruments_env_.taxi_procedure = fpl_->traj()->procedure(fms::PROC_TAXI_1);
    instruments_env_.traj_data = fpl_->traj()->traj_data();
}

void view::on_assigned_fpl_changed()
{
    if (fpl_ = aircraft::info_ptr(parent().lock())->get_fpl())
    {
        if (auto traj = fpl_->traj())
        {
            procedure_ = traj->procedure(fms::PROC_ROUTE);
            instruments_env_.procedure = procedure_;
            instruments_env_.taxi_procedure = traj->procedure(fms::PROC_TAXI_1);
            instruments_env_.traj_data = fpl_->traj()->traj_data();
        }

        procedure_changed_conn_ = fpl_->subscribe_procedure_changed(boost::bind(&view::on_procedure_changed, this));
    }
}


fms::plan_t const& view::get_plan()const
{
    return plan_;
}

fpl::info_ptr view::get_fpl() const
{
    return fpl_;
}

fms::procedure_ptr view::get_procedure() const
{
    return procedure_;
}

fms::instruments::environment const& view::get_instruments_env() const
{
    return instruments_env_;
}

void view::make_aircraft_info()
{
    if (ada_)
    {
        aircraft_data_ = *ada_->get_data(aircraft_kind_);
        procedure_model_ = fms::create_bada_procedure_model(*aircraft_data_);

        instruments_env_.procedure_model = procedure_model_;
        instruments_env_.operation_model = procedure_model_;
        instruments_env_.traj_data.aircraft_data.settings = *aircraft_data_;
        instruments_env_.traj_data.aircraft_data.payload_mass = get_payload_mass();
    }
}

AUTO_REG_NAME(aircraft_fms_view, view::create);

}
}