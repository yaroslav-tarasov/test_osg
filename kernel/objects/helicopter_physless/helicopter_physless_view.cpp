#include "stdafx.h"
#include "precompiled_objects.h"

#include "helicopter_physless_view.h"
#include "helicopter_physless_common.h"

#include "kernel/object_info.h"


namespace helicopter_physless
{

namespace
{

static double const max_airport_search_radius = 2500. ;

} // end of anonymous namespace

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{   
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(helicopter_physless_view, view::create);



view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation    (oc)
    , obj_data_base             (dict)
    , nodes_manager_            (find_first_child<nodes_management::manager_ptr>(this))

    , max_pred_len_             (120.)
    , providers_count_          (0)

{
    if (nodes_manager_)
    {
        if (!settings_.kind.empty() && 
            nodes_manager_->get_model() != aircraft::get_model(settings_.kind)/* &&
            !has_assigned_fpl()*/)
        {
           nodes_manager_->set_model(aircraft::get_model(settings_.kind));
        }

        if (auto tow_point_node = nodes_manager_->find_node("tow_point"))
        {
            auto damned_offset_node  = nodes_manager_->find_node("damned_offset");
            cg::transform_4 tr = nodes_manager_->get_relative_transform(damned_offset_node);
            tow_point_transform_ = nodes_manager_->get_relative_transform(/*nodes_manager_,*/ tow_point_node) * tr;
        }
    }

    void (view::*on_rb) (msg::rotor_state_msg const& msg) = &view::on_rotor_state;

    msg_disp()
        .add<msg::settings_msg      >(boost::bind(&view::on_settings    , this, _1))
        .add<msg::malfunction_msg   >(boost::bind(&view::on_malfunction , this, _1))
        .add<msg::engine_state_msg  >(boost::bind(&view::on_engine_state, this, _1))
        .add<msg::traj_assign_msg   >(boost::bind(&view::on_traj_assign, this, _1))
        
        .add<msg::local_meteo_msg   >(boost::bind(&view::on_local_meteo, this, _1))
        .add<msg::state_msg>         (boost::bind(&view::on_state, this, _1))
        .add<msg::rotor_state_msg    >(boost::bind(on_rb, this, _1))

        // just for recording visual effect to history 
        .add<msg::contact_effect        >(boost::bind(&view::on_contact_effect      , this, _1))
        .add<msg::wheel_contact_effect  >(boost::bind(&view::on_wheel_contact_effect, this, _1))
        ;

    if(dict)
    {
        _state.dyn_state.pos     = state_.pos;
        _state.dyn_state.course  = state_.orien.get_course();
        _state.pitch             = state_.orien.get_pitch();
        _state.roll              = state_.orien.get_roll();


    }

}

view::~view()
{
}

void view::pre_update(double time)
{
    base_view_presentation::pre_update(time);
}

void view::on_object_created(object_info_ptr object)
{
    base_view_presentation::on_object_created(object);
}

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object);

    
}

void view::on_child_appended(object_info_ptr child)
{
    // nothing to do
}

void view::on_child_removing(object_info_ptr child)
{
    base_view_presentation::on_child_removing(child) ;
 
   if (nodes_manager_ == child)
        nodes_manager_.reset() ;
}

geo_point_3 const& view::pos() const
{
    FIXME(position);
    // return return fms_info_->get_state().dyn_state.pos;

    return get_state().dyn_state.pos;

}

cg::point_3 view::dpos() const
{  
    FIXME(dpos);
    return  cg::polar_point_3(get_state().dyn_state.TAS, get_state().orien().course, get_state().orien().pitch);
}

cpr view::orien() const
{   
    FIXME(orien());
    return get_state().orien();
}

void view::set_state(state_t const& state)
{
    set_state(state, true);
}

void view::set_state(state_t const& st, bool sure)
{                   
    set(msg::state_msg(st), sure);
}

aircraft::settings_t const & view::settings() const
{
    return settings_;
}

#if 1
fpl::info_ptr view::get_fpl() const
{
	assert(0);
	return /*fpl_*/fpl::info_ptr();
}
#endif

bool view::has_assigned_fpl() const
{
	assert(0);
	return /*fpl_id_*/false;
}

transform_4 const&  view::tow_point_transform() const
{
    return tow_point_transform_;
}

nodes_management::node_info_ptr view::root() const
{
    return nodes_manager_->get_node(0);
}

nodes_management::node_info_ptr view::tow_point() const
{
    return nodes_manager_->find_node("tow_point");
}

bool view::malfunction(aircraft::malfunction_kind_t kind) const
{
    return malfunctions_[kind];
}



optional<double> view::get_prediction_length() const
{
    return prediction_len_;
}

optional<double> view::get_proc_length() const
{
    return proc_len_;
}


void view::set_kind(std::string const& kind)
{
    aircraft::settings_t s = settings_;
    s.kind = kind;

    set(msg::settings_msg(s));
}

void view::set_turbulence(unsigned turbulence)
{
    aircraft::settings_t s = settings_;
    s.turbulence = turbulence;

    set(msg::settings_msg(s));
}

aircraft::aircraft_fms::info_ptr view::get_fms() const
{
	assert(0);
	return /*fms_info_*/aircraft::aircraft_fms::info_ptr();
}

void view::set_malfunction(aircraft::malfunction_kind_t kind, bool enabled)
{
    set(msg::malfunction_msg(kind, enabled));
}

void view::set_engine_state(aircraft::engine_state_t state)
{
    set(msg::engine_state_msg(state));
}

void view::set_cmd_go_around(uint32_t cmd_id)
{ 
#if 0
	fpl::control_ptr const fpl = get_fpl();
	Assert(fpl);

	fpl->set_cmd_go_around(cmd_id, len_for_cmd());
#endif
}

void view::set_cmd_holding(uint32_t cmd_id, fms::holding_t const &holding)
{
#if 0
	fpl::control_ptr const fpl = get_fpl();
	Assert(fpl);

	fpl->set_cmd_holding(cmd_id, len_for_cmd(), holding);
#endif
}

void view::set_cmd_course(uint32_t cmd_id, fms::course_modifier_t const &course)
{
#if 0
	fpl::control_ptr const fpl = get_fpl();
	Assert(fpl);

	fpl->set_cmd_course(cmd_id, len_for_cmd(), course);
#endif
}

void view::cancel_cmd(uint32_t cmd_id)
{
#if 0
	fpl::control_ptr const fpl = get_fpl();
	Assert(fpl);

	fpl->cancel_cmd(cmd_id);
#endif
}

void view::set_responder_mode(atc::responder_mode mode)
{
	if (settings_.responder.mode == mode)
		return ;

	aircraft::settings_t s = settings_;
	s.responder.mode = mode ;

	set(msg::settings_msg(s));
}

void view::set_responder_type(atc::squawk_type stype)
{
	if (settings_.responder.type == stype)
		return ;

	aircraft::settings_t s = settings_;
	s.responder.type = stype ;

	set(msg::settings_msg(s));
}

void view::set_responder_code(unsigned code)
{
	if (settings_.responder.code == code)
		return ;

	aircraft::settings_t s = settings_;

	if (!old_responder_code_)
		old_responder_code_ = s.responder.code ;

	s.responder.code = code ;
	set(msg::settings_msg(s));
}

void view::set_responder_flag(unsigned flag, bool enable)
{
	if (((settings_.responder.flags & flag) == flag) && enable)
		return ;

	aircraft::settings_t s = settings_;

	if (enable)
		s.responder.flags = flag ;
	else
		s.responder.flags &= ~flag ;

	set(msg::settings_msg(s));
}

void view::restore_responder_code()
{
	if (!old_responder_code_)
		return ;

	aircraft::settings_t s = settings_;

	s.responder.code = *old_responder_code_ ;
	set(msg::settings_msg(s));

	old_responder_code_ = boost::none ;
}

void view::set_parking_initial_position(std::string const &airport_name, std::string const &parking_name)
{
    Assert(nodes_manager_);
    //Assert(fms_info_);
#if 0
    if (ani_ && !airport_name.empty() && !parking_name.empty())
    {
        ani::airport_info_ptr airp = ani_->navigation_info()->get_airport(airport_name);
        if (airp)
        {
            auto park = airp->find_parking(parking_name);
            if (park)
            {
                ani::parking_info const& park_info = airp->get_parking_info(*park);
                point_2 dir = cg::normalized_safe(cg::geo_direction(park_info.end_pos, park_info.pos));
                double initial_course = polar_point_2(dir).course;

                geo_base_3 initial_pos(geo_base_2(park_info.pos)(-dir * fms_info_->length_fwd()), 0);

                nodes_management::node_control_ptr root(nodes_manager_->get_node(0));
                root->set_position(geo_position(initial_pos, quaternion(cpr(initial_course,0,0))));


                aircraft_fms::state_t st = fms_info_->get_state();
                st.dyn_state.pos = initial_pos;
                st.dyn_state.course = initial_course;
                st.dyn_state.TAS = 0;

                if (cg::eq_zero(initial_pos.height))
                    st.dyn_state.cfg = fms::CFG_GD;

                aircraft_fms::control_ptr(fms_info_)->set_state(st);


            }
        }
    }
    else if (parking_name == "F")
    {
        aircraft_fms::state_t st = fms_info_->get_state();
        st.dyn_state.pos.height = 0;
        st.dyn_state.cfg = fms::CFG_GD;
        st.dyn_state.TAS = 0;

        nodes_management::node_control_ptr root(nodes_manager_->get_node(0));
        root->set_position(geo_position(st.dyn_state.pos, quaternion(cpr(st.dyn_state.course,0,0))));

        aircraft_fms::control_ptr(fms_info_)->set_state(st);
    }
#endif
}

void view::on_state(msg::state_msg const& msg)
{
    _state = msg;
    state_changed_signal_();
}

void view::on_settings(aircraft::settings_t const& s)
{
    if (!s.parking.empty() && s.parking != settings_.parking)
        set_parking_initial_position(s.airport, s.parking);

    bool model_changed = false;
    if (s.kind != settings_.kind)
    {
        if (nodes_manager_ && (nodes_manager_->get_model() != aircraft::get_model(s.kind)))
        {
            nodes_manager_->set_model(aircraft::get_model(s.kind));

            if (auto tow_point_node = nodes_manager_->find_node("tow_point"))
                tow_point_transform_ = nodes_manager_->get_relative_transform(/*nodes_manager_,*/ tow_point_node);

            model_changed = true;
        }

    }

    if (s.company_name != settings_.company_name || model_changed)
        if (nodes_manager_)
            nodes_management::node_control_ptr(nodes_manager_->get_node(0))->set_texture(aircraft::get_texture(s.kind, s.company_name));

    settings_ = s; 
    on_settings_changed() ;
}


void view::on_traj_assign(msg::traj_assign_msg const &m)
{
    double len =0;
    if(traj_.get())
        len = traj_->cur_len();

    traj_ = fms::trajectory::create(m.traj);
    traj_->set_cur_len(len);

}

void view::on_malfunction(msg::malfunction_msg const& m)
{
    malfunctions_[m.kind] = m.enabled;
    on_malfunction_changed(m.kind);
}

void view::on_engine_state(msg::engine_state_msg const& m)
{
    engines_state_ = m.state;
    on_engine_state_changed(m.state);
}

void view::on_local_meteo(msg::local_meteo_msg const& m)
{
    lp_.wind_speed = m.wind_speed;
    lp_.wind_azimuth = m.wind_azimuth;
}

void view::on_contact_effect(msg::contact_effect const& eff)      
{
    on_new_contact_effect(eff.time, eff.contacts);
}

void view::on_wheel_contact_effect(msg::wheel_contact_effect const& eff)
{
    on_new_wheel_contact_effect(eff.time, eff.vel, eff.offset);
}

void view::on_rotor_state(msg::rotor_state_msg const& msg)
{
    on_rotor_state(msg.target,msg.speed,static_cast<rotor_state_t>(msg.visible));
}

void view::update_len(double time)
{
    double dt = cg::abs(time - (last_len_calc_ ? *last_len_calc_ : 0));
 #if 0
    if (fpl_ && fpl_->traj())
    {
        double range_size = fms_info_->get_state().dyn_state.TAS * 2. * dt;
        
        optional<range_2> prediction_rng, proc_rng;
        if (prediction_len_)
            prediction_rng = cg::range_2(*prediction_len_).inflate(range_size);
        
        prediction_len_ = fpl_->traj()->get_prediction()->closest(pos(), prediction_rng);

        if (proc_len_)
            proc_rng = cg::range_2(*proc_len_).inflate(range_size);
        
        if (fms::procedure_ptr proc = fpl_->traj()->procedure(fms::PROC_ROUTE))
            proc_len_ = proc->closest(pos(), proc_rng);
    }
#endif
    last_len_calc_ = time;

}

} // end of aircraft
