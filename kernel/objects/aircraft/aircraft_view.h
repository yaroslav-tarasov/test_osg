#pragma once

#include "objects/ani_fwd.h"

#include "common/aircraft.h"
#include "common/aircraft_fms.h"
//#include "common/tp_sys.h"
#include "common/meteo_proxy.h"
//#include "common/conflicts_manager.h"
//#include "common/fpl_manager.h"

#include "objects/nodes_management.h"
//#include "objects/aircraft_gui.h"
#include "aircraft/aircraft_common.h"

namespace aircraft
{
#if 0 // FIXME Need to realize
struct tp_provider_impl
    : tp_provider
{
    tp_provider_impl(boost::function<fms::tp::aircraft_ptr ()> get_tp,
                     boost::function<void ()> on_destroy)
        : get_tp_    (get_tp    )
        , on_destroy_(on_destroy)
    { }

    ~tp_provider_impl()
    {
        on_destroy_();
    }

    fms::model_traj_ptr prediction()
    {
        auto tp = get_tp_();
        return tp ? tp->get_prediction() : fms::model_traj_ptr();
    }

    boost::function<fms::tp::aircraft_ptr ()>  get_tp_;
    boost::function<void ()>                   on_destroy_;
};
#endif

struct craft_data
{
    explicit craft_data(settings_t const& settings = settings_t(), state_t const& state = state_t(),obj_id_opt fpl_id = boost::none)
        : settings_     (settings)
        , state_        (state)
        , fpl_id_       (fpl_id)
    {
        std::for_each(malfunctions_.begin(), malfunctions_.end(), [](bool& item){item = false;});
    }

protected:
    obj_id_opt              fpl_id_;
    settings_t              settings_;
    array<bool, MF_SIZE>    malfunctions_;
    atc_controls_t          atc_controls_;
    ipo_controls_t          ipo_controls_;
    state_t                 state_;       // Исключительно для задания начальных параметров 
    REFL_INNER(craft_data)
        REFL_ENTRY(fpl_id_      )
        REFL_ENTRY(settings_    )
        REFL_ENTRY(malfunctions_)
        REFL_ENTRY(atc_controls_)
        REFL_ENTRY(ipo_controls_)
        REFL_ENTRY(state_       )
    REFL_END()
};


//////////////////////////////////////////////////////////////////////////

struct view
    : kernel::base_view_presentation
    , obj_data_holder<craft_data>
    , info
#if 0
    , ani_info
#endif
    , control
    , aircraft_ipo_control
    , aircraft_atc_control
    , fms_container
    , atc_info
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    typedef 
        msg::contact_effect::contact_t 
        contact_t;

protected:
    view( kernel::object_create_t const& oc, dict_copt dict );

public:
    ~view();

    // base_presentation
protected:
    void pre_update(double time) override;

    // base_view_presentation
protected:
    void on_object_created   (object_info_ptr object) override ;
    void on_object_destroying(object_info_ptr object) override ;
    void on_child_appended   (object_info_ptr child)  override ;
    void on_child_removing   (object_info_ptr child)  override ;

    // info
protected:
    geo_point_3 const&  pos                () const override;
    point_3             dpos               () const override;
    cpr                 orien              () const override;
    settings_t const &  settings           () const override;
#if 1
    fpl::info_ptr       get_fpl            () const override;
#endif
    bool                has_assigned_fpl   () const override;
    transform_4 const&  tow_point_transform() const override;

    nodes_management::node_info_ptr root() const override; 
    nodes_management::node_info_ptr tow_point() const override;

    bool malfunction(malfunction_kind_t kind) const override;
#if 0
    tp_provider_ptr get_tp_provider(double duration_sec) override;

    atc_controls_t const&     get_atc_controls() const override;
    ipo_controls_t const&     get_ipo_controls() const override;
    aircraft_gui::control_ptr get_gui()          const override;
#endif
    optional<double> get_prediction_length() const override;
    optional<double> get_proc_length() const override;

    // ani_info
#if 0
protected:
    std::string current_airport() const override;
    std::string next_airport()    const override;
    std::string prev_airport()    const override;

    optional<double> time_to_first_instrument(fms::instrument_kind_t instr) const override;

    ::ani::area_t    current_area() const override;
    ::ani::area_t    next_area()    const override;
    ::ani::area_t    prev_area()    const override;

    boost::optional<double> time_to_inarea(::ani::area_t const &area)  const override;
    boost::optional<double> time_to_outarea(::ani::area_t const &area) const override;

    boost::optional<double> in_area_len(::ani::area_t const &area)  const override;
    boost::optional<double> out_area_len(::ani::area_t const &area)  const override;
    cg::range_2 area_len_range(::ani::area_t const &area)  const override;

    std::vector<uint32_t>  sectors_sequence() const override ;
#endif

    // control
protected:
#if 0
    void assign_fpl         (fpl::info_ptr fpl_obj);
#endif
    void unassign_fpl       ();
    void set_kind           (std::string const& kind) override;
    void set_turbulence     (unsigned turb) override;
#if 0
    void set_atc_controls   (atc_controls_t const& controls) override;
    void set_ipo_controls   (ipo_controls_t const& controls) override;
#endif

    // aircraft_ipo_control
protected:
    void set_malfunction(malfunction_kind_t kind, bool enabled) override;
    void set_cmd_go_around(uint32_t cmd_id) override;
    void set_cmd_holding  (uint32_t cmd_id, fms::holding_t const &holding)   override;
    void set_cmd_course   (uint32_t cmd_id, fms::course_modifier_t const &course)   override;
    void cancel_cmd       (uint32_t cmd_id)   override;

    void set_responder_mode(atc::responder_mode mode) override;
    void set_responder_type(atc::squawk_type stype) override;
    void set_responder_code(unsigned code) override;
    void set_responder_flag(unsigned flag, bool enable) override;
    void restore_responder_code() override;

protected:
    void set_parking_initial_position(std::string const &airport_name, std::string const &parking_name);

    // aircraft_atc_control
private:
    void change_speed_display(boost::optional<double> value) override {}
    void change_dist_display (boost::optional<double> value) override {}
    void set_new_callsign(boost::optional<std::string>) override {};
    void activate () override ;

    // fms_container
protected:
    aircraft_fms::info_ptr get_fms() const override;

    // atc_info
protected:
    atc_state_t get_atc_state() const override;
    void        set_atc_state(atc_state_t const &astate) override;
    bool        ssr_synchronized() const override;
    boost::optional<std::string> get_new_callsign() const override;
    std::vector< std::pair<double, std::string> > get_oldi_history() const override;

protected:
    virtual void on_settings_changed() {}
    virtual void on_fpl_changed();
    virtual void on_fms_changed();
    virtual void on_fms_state_changed();
    virtual void on_plan_changed();
    virtual void on_atc_controls_changed() {}
    virtual void on_ipo_controls_changed() {}
    virtual void on_malfunction_changed (malfunction_kind_t /*kind*/) {}
    virtual void on_assigned_fpl_changed();
    virtual void on_new_contact_effect      (double /*time*/, std::vector<contact_t> const& /*contacts*/){}
    virtual void on_new_wheel_contact_effect(double /*time*/, point_3f /*vel*/, point_3f /*offset*/){}
    
private:
    void                   refresh_tp();
#if 0
    fms::tp::aircraft_ptr  give_tp_to_provider() const;
#endif
    void                   on_provider_destroy();
    double                 len_for_cmd() const;

    // msg dispatching
private:
    void on_settings            (settings_t const&          m);
    void on_fpl                 (optional<uint32_t> const&  id);

    void on_atc_state           (msg::atc_state_msg     const&  m);
    void on_malfunction         (msg::malfunction_msg   const&  m);
    void on_atc_controls        (msg::atc_controls_msg  const& controls);
    void on_ipo_controls        (msg::ipo_controls_msg  const& controls);
    void on_contact_effect      (msg::contact_effect    const& eff)     ;
    void on_wheel_contact_effect(msg::wheel_contact_effect const& eff)  ;

protected:
    void set_fpl_initial_position();
    void update_atc_state();
    
    nodes_management::manager_ptr  get_nodes_manager() const { return nodes_manager_; }
   
    aircraft_fms::info_ptr         get_fms_info() const { return fms_info_; }
#if 0
    conflicts_manager::control_ptr get_conflicts_manager() const { return conflicts_manager_; }
#endif

private:
    optional<double> last_len_calc_;

private:
#if 0 
    conflicts_manager::control_ptr conflicts_manager_;
#endif
    nodes_management::manager_ptr  nodes_manager_;
    aircraft_fms::info_ptr         fms_info_;


private:
#if 0
    aircraft_gui::control_ptr      gui_;
#endif

private:
    scoped_connection fms_changed_connection_ ;
    scoped_connection state_changed_connection_ ;
    scoped_connection plan_changed_connection_ ;

protected:
    transform_4 tow_point_transform_;

protected:
    fpl::info_ptr       fpl_ ;
 
	optional<double>    proc_len_;
    optional<double>    prediction_len_;

    void update_len(double time);

protected:
#if 1 
    ani_object::info_ptr    ani_;
#endif
    meteo_proxy::info_ptr   met_proxy_obj_;
    meteo::meteo_proxy_ptr  meteo_proxy_;

private:
#if 0 
    tp_sys::control_ptr    tp_sys_;
    fms::tp::aircraft_ptr  airc_tp_;
#endif

    size_t                 providers_count_;
    double                 max_pred_len_;

private:
    atc_state_t               atc_state_ ;
    boost::optional<uint32_t> state_arm_id_ ;

private:
    boost::optional<unsigned> old_responder_code_ ;

private:
    scoped_connection fpl_changed_conn_;

private:
    std::vector< std::pair<double, std::string> > oldi_sequence_ ;

};

} // aircraft
