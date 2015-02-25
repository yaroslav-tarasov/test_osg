#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_view.h"
#include "aircraft_common.h"

#include "kernel/object_info.h"
#if 0
#include "fms/traj_calc.h"
#include "fms/fms_instruments.h"
#include "ani/ani.h"

#include "objects/ani.h"


#include "common/fpl.h"
#endif

namespace aircraft
{

namespace
{

static double const max_airport_search_radius = 2500. ;
#if 0
void fill_air_settings_from_fpl(aircraft::settings_t &air_sett, fpl::settings_t const &fpl_sett, bool skip_ssr)
{
    air_sett.kind         = fpl_sett.aircraft_type;
    air_sett.company_name = fpl_sett.company_code;
    air_sett.payload      = fpl_sett.payload;
    air_sett.fuelload     = fpl_sett.fuelload;
    air_sett.turbulence   = fpl_sett.turbulence;
    air_sett.equipment    = fpl_sett.equipment;

    if (!skip_ssr)
        air_sett.responder.code = fpl_sett.ssr_code;
}

std::string make_OLDI_msg(aircraft::atc_state_t atc_state)
{
    switch (atc_state)
    {
    case AS_NOT_CONCERNED_ABI :
        return "ABI RECV LAM ABI ";
    case AS_ACTIVATED :
        return "ACT RECV LAM ACT";
    }
    return "" ;
}
#endif
} // end of anonymous namespace

object_info_ptr view::create(kernel::object_create_t const& oc/*, dict_copt dict*/)
{
    return object_info_ptr(new view(oc/*, dict*/));
}

//AUTO_REG_NAME(aircraft_view, view::create);



view::view(kernel::object_create_t const& oc/*, dict_copt dict*/)
    : base_view_presentation    (oc)
    //, obj_data_base             (dict)
#if 0
    , conflicts_manager_        (find_first_object<conflicts_manager::control_ptr>(collection_))
#endif    
    , nodes_manager_            (find_first_child<nodes_management::manager_ptr>(this))
    , fms_info_                 (find_first_child<aircraft_fms::info_ptr       >(this))
#if 0
    , gui_                      (find_first_child<aircraft_gui::control_ptr    >(this))
#endif
#if 0    
    , ani_                      (find_first_object<ani_object::info_ptr>(collection_))
    , met_proxy_obj_            (find_first_object<meteo_proxy::info_ptr>(collection_))
    , meteo_proxy_              (met_proxy_obj_->get_general_proxy())
    , tp_sys_                   (find_first_object<tp_sys::control_ptr >(collection_))
#endif
    , max_pred_len_             (120.)
#if 0 
    , fms_changed_connection_   (fms_info_->subscribe_fms_changed  (boost::bind(&view::on_fms_changed      , this)))
    , plan_changed_connection_  (fms_info_->subscribe_plan_changed (boost::bind(&view::on_plan_changed      , this)))
    , state_changed_connection_ (fms_info_->subscribe_state_changed(boost::bind(&view::on_fms_state_changed, this)))
#endif
    , providers_count_          (0)

    , atc_state_                (AS_NOT_CONCERNED)
{
    if (nodes_manager_)
    {
        if (!settings_.kind.empty() && 
            nodes_manager_->get_model() != get_model(settings_.kind) &&
            !has_assigned_fpl())
        {
           nodes_manager_->set_model(get_model(settings_.kind));
        }

        if (auto tow_point_node = nodes_manager_->find_node("tow_point"))
            tow_point_transform_ = get_relative_transform(nodes_manager_, tow_point_node);
    }

#if 0 
    conflicts_manager_->register_aircraft(this);
#endif

    msg_disp()
        .add<msg::settings_msg      >(boost::bind(&view::on_settings    , this, _1))
        .add<msg::fpl_msg           >(boost::bind(&view::on_fpl         , this, _1))
        .add<msg::atc_state_msg     >(boost::bind(&view::on_atc_state   , this, _1))
        .add<msg::malfunction_msg   >(boost::bind(&view::on_malfunction , this, _1))
        .add<msg::atc_controls_msg  >(boost::bind(&view::on_atc_controls, this, _1))
        .add<msg::ipo_controls_msg  >(boost::bind(&view::on_ipo_controls, this, _1))
        
        // just for recording visual effect to history 
        .add<msg::contact_effect        >(boost::bind(&view::on_contact_effect      , this, _1))
        .add<msg::wheel_contact_effect  >(boost::bind(&view::on_wheel_contact_effect, this, _1));
}

view::~view()
{
#if 0 
    conflicts_manager_->unregister_aircraft(this);
#endif
}

void view::pre_update(double time)
{
    base_view_presentation::pre_update(time);
#if 0 
    if (airc_tp_)
        airc_tp_->set_state(fms_info_->get_state());
#endif
}

void view::on_object_created(object_info_ptr object)
{
    base_view_presentation::on_object_created(object);
#if 0 
    // If FPL created and should be assigned or this was created and could be assigned
    if (fpl_id_ && (*fpl_id_ == object->object_id() || this == object.get()))
    {
        on_fpl(fpl_id_);

        if (fpl_)
            ani_info_changed_signal_() ;
    }

    if (!airc_tp_ && fms_info_ && fpl_ && fpl_->traj() && fpl_->traj()->procedure(fms::PROC_ROUTE))
        refresh_tp();
#endif
}

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object);

    // If FPL is destroying and should be unassigned or this is about to destroy and could be unassigned
#if 0     
    if (fpl_id_ && (fpl_ == object || this == object.get()))
    {
        on_fpl(boost::none);

        if (!fpl_)
            ani_info_changed_signal_() ;
    }
#endif
}

void view::on_child_appended(object_info_ptr child)
{
    // nothing to do
}

void view::on_child_removing(object_info_ptr child)
{
    base_view_presentation::on_child_removing(child) ;
#if 0  
    if (fms_info_ == child)
        fms_info_.reset() ;
    else if (nodes_manager_ == child)
        nodes_manager_.reset() ;
    else if (gui_ == child)
        gui_.reset() ;
#endif
}

geo_point_3 const& view::pos() const
{
    // FIXME  
    FIXME(Здесь должен быть позицион)
    return geo_point_3();// return fms_info_->get_state().dyn_state.pos;
}

point_3 view::dpos() const
{
    // FIXME
    FIXME(Здесь должен быть позицион)
    return point_3();
    // return cg::polar_point_3(fms_info_->get_state().dyn_state.TAS, fms_info_->get_state().orien().course, fms_info_->get_state().orien().pitch);
}

cpr view::orien() const
{   
    // FIXME  
    FIXME(Здесь должена быть ориентация)
    return cpr();
    // return fms_info_->get_state().orien();
}

settings_t const & view::settings() const
{
    return settings_;
}
#if 1
fpl::info_ptr view::get_fpl() const
{
    return fpl_;
}
#endif

bool view::has_assigned_fpl() const
{
    return fpl_id_;
}

transform_4 const&  view::tow_point_transform() const
{
    return tow_point_transform_;
}

nodes_management::node_info_ptr view::root() const
{
    return nodes_manager_->get_node(0);
}

bool view::malfunction(malfunction_kind_t kind) const
{
    return malfunctions_[kind];
}
 #if 0
tp_provider_ptr view::get_tp_provider(double duration_sec)
{
    if (duration_sec > max_pred_len_)
        max_pred_len_ = duration_sec;

    ++providers_count_;
    refresh_tp();
    if (airc_tp_ && airc_tp_->get_duration() < duration_sec)
        airc_tp_->set_duration(max_pred_len_);

    return tp_provider_ptr(new tp_provider_impl(boost::bind(&view::give_tp_to_provider, this),
                                                boost::bind(&view::on_provider_destroy, this)));
}

/*
bool view::can_go_around() const 
{
    fms::traj_collection_ptr traj = fpl_->traj();
    if (!traj->plan_traj().attr.around_route)
        return false;

    Assert(traj->tactical_traj().attr.faf_len);
    const double len = len_for_cmd();
    const double faf_len = *traj->tactical_traj().attr.faf_len;

    return (cg::gt(len, faf_len, 10.));
}
*/

atc_controls_t const& view::get_atc_controls() const
{
    return atc_controls_;
}

ipo_controls_t const& view::get_ipo_controls() const
{
    return ipo_controls_;
}

aircraft_gui::control_ptr view::get_gui() const
{
    return gui_;
}
#endif

optional<double> view::get_prediction_length() const
{
    return prediction_len_;
}

optional<double> view::get_proc_length() const
{
    return proc_len_;
}

boost::optional<std::string> view::get_new_callsign() const 
{
    return boost::none;
}

std::vector< std::pair<double, std::string> > view::get_oldi_history() const 
{
    return oldi_sequence_ ;
}
#if 0
std::string view::current_airport() const
{
    if (!ani_)
        return std::string() ;

    ani::navigation_ptr navi = ani_->navigation_info() ;

    // TODO : decide if aircraft is on the ground according to real height
    if (navi && (pos().height < 10.))
        if (ani::airport_info_ptr port = navi->find_airport(pos(), max_airport_search_radius))
            return port->name() ;

    return std::string() ;
}

std::string view::next_airport() const
{
    if (!fpl_ || !fpl_->traj() || !ani_)
        return std::string() ;

    return fpl_->traj()->traj_data().attr.dest_airport;
}

std::string view::prev_airport() const
{
    if (!fpl_ || !fpl_->traj() || !ani_)
        return std::string() ;

    return fpl_->traj()->traj_data().attr.depart_airport;
}
#endif
// std::string view::next_rw() const // TODO: add RW
// {
//     return "25_L" ;
// }
#if 0
optional<double> view::time_to_first_instrument(fms::instrument_kind_t kind) const 
{
    if (!fpl_)
        return none;
    
    Assert(fpl_->traj() && fpl_->traj()->get_prediction());
    if (!fpl_->traj() || !fpl_->traj()->get_prediction())
        return none;

    if (!fpl_->traj_navi())
        return none;

    double prediction_len = prediction_len_.get_value_or(0.);

    fms::model_traj_ptr prediction = fpl_->traj()->get_prediction();
    
    vector<double> const &instr_start_lengths = fpl_->traj()->instr_start_lenghts();
    if (instr_start_lengths.empty())
        return none;

    fms::plan_t const &plan = fpl_->traj()->get_initial_plan();
    Assert(instr_start_lengths.size() == plan.size());
    
    for (size_t i = 0; i < plan.size(); ++i)
    {
        if (plan.at(i)->kind() == kind)
        {
            double len = instr_start_lengths.at(i);
            return prediction->time(len) - prediction->time(prediction_len);
        }
    }

    if (kind == fms::INSTRUMENT_BRAKING)
    {
        if (!fpl_->traj()->traj_data().attr.dest_airport.empty())
        {
            double len = prediction->length();
            return prediction->time(len) - prediction->time(prediction_len);
        }
    }

    return none;
}
#endif

#if 0 
::ani::area_t view::current_area() const
{
    ::ani::area_t area ;

    if (!ani_)
        return area ;

    ani::contours_info_ptr contours = ani_->get_contours() ;
    std::vector<size_t> sectors = std::move(contours->get_contours(pos())) ;

    for (auto it = sectors.begin(), end = sectors.end(); it != end; ++it)
    {
        ani::contour_info const& info = contours->get_contour(*it);

        if (info.kind() == ani::SECTOR)
        {
            ani::atc_sector_data const& atc_data = info.as_sector() ;

            if (atc_data.fl_range.contains(pos().height))
                area.insert(*it) ;
        }
    }

    return area ;
}

::ani::area_t view::next_area() const
{
    return (fpl_ && prediction_len_ && fpl_->traj_navi()) ? fpl_->traj_navi()->next_area(*prediction_len_) : ::ani::area_t() ;
}

::ani::area_t view::prev_area() const
{
    return (fpl_ && prediction_len_ && fpl_->traj_navi()) ? fpl_->traj_navi()->prev_area(*prediction_len_) : ::ani::area_t() ;
}

boost::optional<double> view::time_to_inarea(::ani::area_t const &area) const
{
    if (fpl_ && fpl_->traj() && prediction_len_ && fpl_->traj_navi())
    {
        if (boost::optional<double> len = fpl_->traj_navi()->copin(area))
        {
            auto proc = fpl_->traj()->get_prediction() ;

            return proc->time(*len) - proc->time(*prediction_len_) ;
        }
    }

    return boost::none ;
}

boost::optional<double> view::time_to_outarea(::ani::area_t const &area) const
{
    if (fpl_ && fpl_->traj() && prediction_len_&& fpl_->traj_navi())
    {
        if (boost::optional<double> len = fpl_->traj_navi()->copout(area))
        {
            Assert(fpl_->traj()) ;
            auto proc = fpl_->traj()->get_prediction() ;

            return proc->time(*len) - proc->time(*prediction_len_) ;
        }
    }

    return boost::none ;
}

boost::optional<double> view::in_area_len(::ani::area_t const &area)  const
{
    if (fpl_ && fpl_->traj() && prediction_len_ && fpl_->traj_navi())
    {
        return fpl_->traj_navi()->copin(area);
    }

    return boost::none ;
}

boost::optional<double> view::out_area_len(::ani::area_t const &area)  const
{
    if (fpl_ && fpl_->traj() && prediction_len_ && fpl_->traj_navi())
    {
        return fpl_->traj_navi()->copout(area);
    }
    return boost::none ;
}

cg::range_2 view::area_len_range(::ani::area_t const &area)  const
{
    if (fpl_ && fpl_->traj() && prediction_len_ && fpl_->traj_navi())
    {
        optional<double> in_len = fpl_->traj_navi()->copin(area);
        optional<double> out_len = fpl_->traj_navi()->copout(area);

        cg::range_2 r(0., *prediction_len_);
        if (in_len)
            r = cg::range_2(cg::max(r.lo(), *in_len), r.hi());
        if (out_len)
            r = cg::range_2(r.lo(), cg::min(r.hi(), *out_len));

        return r;
    }

    return cg::range_2();
}


std::vector<uint32_t>  view::sectors_sequence() const
{
    if (fpl_ && fpl_->traj() && prediction_len_ && fpl_->traj_navi())
    {
        return fpl_->traj_navi()->sectors_sequence();
    }

    return std::vector<uint32_t>();
}
#endif
#if 0 
void view::assign_fpl(fpl::info_ptr fpl_obj)
{ 
    set(msg::fpl_msg(object_info_ptr(fpl_obj)->object_id()));
}
#endif

void view::unassign_fpl()
{
    set(msg::fpl_msg(boost::none));
}

void view::set_kind(std::string const& kind)
{
    settings_t s = settings_;
    s.kind = kind;

    set(msg::settings_msg(s));
}

void view::set_turbulence(unsigned turbulence)
{
    settings_t s = settings_;
    s.turbulence = turbulence;

    set(msg::settings_msg(s));
}

void view::activate ()
{
    set_atc_state(AS_ACTIVATED);
}

#if 1
aircraft_fms::info_ptr view::get_fms() const
{
    return fms_info_;
}
#endif

atc_state_t view::get_atc_state() const
{
    return atc_state_;
}

void view::set_atc_state(atc_state_t const &astate)
{
    if (atc_state_ != astate)
        set(msg::atc_state_msg(astate)) ;
}

bool view::ssr_synchronized() const
{
#if 0
    return fpl_ && (settings_.responder.code == fpl_->settings().ssr_code) || (atc::RM_OFF == settings_.responder.mode) ;
#endif
    return false;
}

void view::on_fpl_changed()
{
#if 0
    Assert(fpl_);

    settings_t s = settings();
    fill_air_settings_from_fpl(s, fpl_->settings(), true);

    Assert(fpl_->traj());
    auto procedure = fpl_->traj()->get_prediction();
    
    prediction_len_.reset();
    if (procedure)
        prediction_len_ = procedure->closest(pos());

    refresh_tp();
    set(msg::settings_msg(s));
#endif
}

void view::on_fms_changed()
{
    refresh_tp();
}

void view::on_fms_state_changed()
{
#if 0
    if (fms_info_->get_state().version == 0)
        update_atc_state() ;
#endif

    // TODO
//     if (prev_flight_state_ != fms_info_->get_state().flight_state)
//     {
//         if (fms::FS_INACTIVE == prev_flight_state_ || fms::FS_ENDED == prev_flight_state_)
//         {
//             if (fms::FS_INACTIVE != fms_info_->get_state().flight_state && fms::FS_ENDED != fms_info_->get_state().flight_state)
//                 refresh_tp();
//         }
// 
//         prev_flight_state_ = fms_info_->get_state().flight_state;
//     }
}

void view::on_plan_changed()
{
    refresh_tp();
    update_atc_state();
}

void view::on_assigned_fpl_changed()
{
#if 0
    if (fpl_)
    {
        fpl_changed_conn_ = fpl_->subscribe_changed(boost::bind(&view::on_fpl_changed, this));
        
        if (auto procedure = fpl_->traj()->get_prediction())
            prediction_len_ = procedure->closest(pos());
        else 
            prediction_len_ = boost::none;

        set_fpl_initial_position();
    }
    else
    {
        fpl_changed_conn_ = connection();
        prediction_len_   = boost::none ;
    }
#endif
    last_len_calc_ = boost::none ;
}

void view::refresh_tp()
{
    if (0 == providers_count_)
        return;
#if 0
    if (!tp_sys_ || !fms_info_ || !fpl_ || !fpl_->traj() || !fpl_->traj()->procedure(fms::PROC_ROUTE) || !meteo_proxy_)
        return;

    ani::navigation_ptr navi = ani_ ? ani_->navigation_info() : ani::navigation_ptr();

    double const payload_mass = settings_.payload * fms_info_->fsettings()->max_payload_mass;
    fms::aircraft_data_t adata(*(fms_info_->fsettings()), payload_mass);
    airc_tp_ = tp_sys_->get_aircraft_tp(meteo_proxy_, navi, fms_info_->get_state(), fms_info_->get_plan(), adata, max_pred_len_);
#endif
}

#if 0
fms::tp::aircraft_ptr view::give_tp_to_provider() const
{
    return airc_tp_;
}
#endif

void view::on_provider_destroy()
{ 
#if 0
    --providers_count_;
    if (providers_count_ == 0)
        airc_tp_.reset();
#endif
}

#if 0
void view::set_atc_controls(atc_controls_t const& controls)
{
    set(msg::atc_controls_msg(controls));
}

void view::set_ipo_controls(ipo_controls_t const& controls)
{
    set(msg::ipo_controls_msg(controls));
}
#endif

void view::set_malfunction(malfunction_kind_t kind, bool enabled)
{
    set(msg::malfunction_msg(kind, enabled));
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

    settings_t s = settings_;
    s.responder.mode = mode ;

    set(msg::settings_msg(s));
}

void view::set_responder_type(atc::squawk_type stype)
{
    if (settings_.responder.type == stype)
        return ;

    settings_t s = settings_;
    s.responder.type = stype ;

    set(msg::settings_msg(s));
}

void view::set_responder_code(unsigned code)
{
    if (settings_.responder.code == code)
        return ;

    settings_t s = settings_;

    if (!old_responder_code_)
        old_responder_code_ = s.responder.code ;

    s.responder.code = code ;
    set(msg::settings_msg(s));
}

void view::set_responder_flag(unsigned flag, bool enable)
{
    if (((settings_.responder.flags & flag) == flag) && enable)
        return ;

    settings_t s = settings_;

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

    settings_t s = settings_;

    s.responder.code = *old_responder_code_ ;
    set(msg::settings_msg(s));

    old_responder_code_ = boost::none ;
}

void view::set_parking_initial_position(std::string const &airport_name, std::string const &parking_name)
{
    Assert(nodes_manager_);
#if 0
    Assert(fms_info_);
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

double view::len_for_cmd() const
{
    return prediction_len_.get_value_or(0.);
}

void view::on_settings(settings_t const& s)
{
    if (!s.parking.empty() && s.parking != settings_.parking)
        set_parking_initial_position(s.airport, s.parking);

    bool model_changed = false;
    if (s.kind != settings_.kind)
    {
        if (nodes_manager_ && (nodes_manager_->get_model() != get_model(s.kind)))
        {
            nodes_manager_->set_model(get_model(s.kind));

            if (auto tow_point_node = nodes_manager_->find_node("tow_point"))
                tow_point_transform_ = get_relative_transform(nodes_manager_, tow_point_node);

            model_changed = true;
        }
#if 0
        if (fms_info_)
            aircraft_fms::control_ptr(fms_info_)->set_aircraft_kind(s.kind);
        #endif
    }
#if 0
    if (s.payload != settings_.payload && fms_info_)
            aircraft_fms::control_ptr(fms_info_)->set_payload(s.payload);
#endif
    if (s.company_name != settings_.company_name || model_changed)
        if (nodes_manager_)
            nodes_management::node_control_ptr(nodes_manager_->get_node(0))->set_texture(get_texture(s.kind, s.company_name));
#if 0
    if (s.responder != settings_.responder && get_fpl())
        responder_changed_signal_(get_fpl().get());
#endif
    settings_ = s; 
    on_settings_changed() ;
}

void view::on_fpl(optional<uint32_t> const& id)
{
#if 0
    if (fpl_id_ && (!id || (*id != *fpl_id_)))
    {
        Assert(fpl_);

        if (fpl_->assigned())
            fpl::control_ptr(fpl_)->unassign_aircraft();

        fpl_.reset() ;
    }

    if (id)
    {
        if (fpl_ = collection_->get_object(*id))
        {
            fpl::control_ptr(fpl_)->assign_aircraft(object_id());

            settings_t s = settings_;
            fill_air_settings_from_fpl(s, fpl_->settings(), false) ;
            set(msg::settings_msg(s));
        }
    }
    
    fpl_id_ = id;

    if (fpl_id_ && fpl_ || !fpl_id_)
    {
        on_assigned_fpl_changed();
        assigned_fpl_changed_signal_(fpl_);
    }
#endif
}

void view::on_atc_state(msg::atc_state_msg const &m)
{
    Verify(atc_state_ != m.agg) ;

    atc_state_ = m.agg ;

    atc_state_changed_signal_(atc_state_) ;

    Assert(oldi_sequence_.size() < 100) ;
#if 0
    oldi_sequence_.push_back(make_pair((last_len_calc_ ? *last_len_calc_: 0. ), make_OLDI_msg(atc_state_))) ;
#endif
}

void view::on_malfunction(msg::malfunction_msg const& m)
{
    malfunctions_[m.kind] = m.enabled;
    on_malfunction_changed(m.kind);
}

void view::on_atc_controls(msg::atc_controls_msg const& controls)
{
    atc_controls_ = controls;
    on_atc_controls_changed();
}

void view::on_ipo_controls(msg::ipo_controls_msg const& controls)
{
    ipo_controls_ = controls;
    on_ipo_controls_changed();
}

void view::on_contact_effect(msg::contact_effect const& eff)      
{
    on_new_contact_effect(eff.time, eff.contacts);
}

void view::on_wheel_contact_effect(msg::wheel_contact_effect const& eff)
{
    on_new_wheel_contact_effect(eff.time, eff.vel, eff.offset);
}

void view::set_fpl_initial_position()
{
    FIXME(Начальная позиция из fpl)
#if 0  // FIXME  а тут начальная позиция?

    Assert(fpl_) ;

    aircraft_fms::state_t initial_state(fpl_->get_initial_state(), 0, 0, 0);

    if (nodes_manager_)
    {
        nodes_management::node_control_ptr root(nodes_manager_->get_node(0));
        root->set_position(geo_position(initial_state.dyn_state.pos, quaternion(initial_state.orien())));
    }

    if (fms_info_)
        aircraft_fms::control_ptr(fms_info_)->set_state(initial_state);
#endif
    prediction_len_ = 0 ;
}

void view::update_atc_state()
{ 
#if 0    
    if (!fpl_ || !ssr_synchronized())
        return;

    if (!get_fms()->get_instrument())
    {
        set_atc_state(AS_NOT_CONCERNED);
        return;
    }

    //если все OLDI выставлены
    auto st = get_atc_state();
    if (st >= AS_ACTIVATED )
        return;

    std::vector<uint32_t> sectors;
    if (ani::traj_navigation_ptr traj_navi = fpl_->traj_navi())
        sectors = traj_navi->sectors_sequence();

    ::ani::area_t cur_area = current_area();

    if (!cur_area) //TODO: think twice
    {
        if (sectors.empty())
            return ;

        ani::area_t area ;
        area.insert(sectors.front()) ;

        if (boost::optional<double> time_in = time_to_inarea(area))
        {
            double time = (*time_in) / 60. ;
            if (time > 20.)
            {
                //если вне всех секторов, >20 мин до входа в первый 
                set_atc_state(AS_NOT_CONCERNED);
            }
            else if (cg::le(time, 20.) && time > 10.)
            {   
                //если вне всех секторов, 10 < t <= 20 мин до входа в первый
                set_atc_state(AS_NOT_CONCERNED_ABI);
            }
            else if (cg::le(time, 10.))
            {
                //если вне всех секторов, 0 < t <= 10 мин до входа в первый
                set_atc_state(AS_ACTIVATED);
            }
        }
    }
    else if (get_atc_state() <= AS_ACTIVATED)
    {
        set_atc_state(AS_ACTIVATED);
    }
#endif
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
