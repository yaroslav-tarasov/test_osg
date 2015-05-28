#pragma once


#include "common/aircraft.h"


#include "objects/nodes_management.h"
#include "aircraft_physless/aircraft_physless_common.h"

namespace aircraft_physless
{

struct craft_data
{
    explicit craft_data(aircraft::settings_t const& settings = aircraft::settings_t(), aircraft::state_t const& state = aircraft::state_t())
        : settings_     (settings)
        , state_        (state)
    {
        std::for_each(malfunctions_.begin(), malfunctions_.end(), [](bool& item){item = false;});
    }

protected:
    aircraft::settings_t              settings_;
    array<bool, aircraft::MF_SIZE>    malfunctions_;
    aircraft::state_t                 state_;       // Исключительно для задания начальных параметров 
    REFL_INNER(craft_data)
        REFL_ENTRY(settings_    )
        REFL_ENTRY(malfunctions_)
        REFL_ENTRY(state_       )
    REFL_END()
};

struct info
{
    virtual ~info(){}

    virtual geo_point_3 const&              pos              () const = 0;
    virtual point_3                         dpos             () const = 0;
    virtual cpr                             orien            () const = 0;
    virtual aircraft::settings_t const &    settings         () const = 0;
    //virtual fpl::info_ptr                   get_fpl          () const = 0;
    //virtual bool                            has_assigned_fpl () const = 0;

    virtual transform_4 const&              tow_point_transform() const = 0;
    virtual nodes_management::node_info_ptr tow_point        () const = 0;

    virtual nodes_management::node_info_ptr root             () const = 0;
    virtual bool                            malfunction      (aircraft::malfunction_kind_t kind) const = 0;

    //virtual aircraft_fms::info_ptr          get_fms          () const = 0;

    virtual optional<double> get_prediction_length() const = 0;
    virtual optional<double> get_proc_length      () const = 0;

    DECLARE_EVENT(assigned_fpl_changed, (fpl::info_ptr));
    DECLARE_EVENT(responder_changed, (fpl::info*));
};

struct control
{
    virtual ~control(){}
    virtual void unassign_fpl() = 0;
    //
    virtual void set_kind(std::string const& kind) = 0;
    virtual void set_turbulence(unsigned turb) = 0;
};


//////////////////////////////////////////////////////////////////////////

struct view
    : kernel::base_view_presentation
    , obj_data_holder<craft_data>
    , info
    , control
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
    aircraft::settings_t const &  settings           () const override;
    //bool                has_assigned_fpl   () const override;
    transform_4 const&  tow_point_transform() const override;

    nodes_management::node_info_ptr root() const override; 
    nodes_management::node_info_ptr tow_point() const override;

    bool        malfunction(aircraft::malfunction_kind_t kind) const override;

    optional<double>    get_prediction_length() const override;
    optional<double>    get_proc_length() const override;


    // control
protected:
    void unassign_fpl()     {};
    void set_kind           (std::string const& kind) override;
    void set_turbulence     (unsigned turb) override;

    // aircraft_ipo_control
protected:
    void set_malfunction(aircraft::malfunction_kind_t kind, bool enabled) /*override*/;

protected:
    void set_parking_initial_position(std::string const &airport_name, std::string const &parking_name);



protected:
    virtual void on_settings_changed() {}
    //virtual void on_fpl_changed();
    //virtual void on_fms_changed();
    //virtual void on_fms_state_changed();
    //virtual void on_plan_changed();
    virtual void on_atc_controls_changed() {}
    virtual void on_ipo_controls_changed() {}
    virtual void on_malfunction_changed (aircraft::malfunction_kind_t /*kind*/) {}
    //virtual void on_assigned_fpl_changed();
    virtual void on_new_contact_effect      (double /*time*/, std::vector<contact_t> const& /*contacts*/){}
    virtual void on_new_wheel_contact_effect(double /*time*/, point_3f /*vel*/, point_3f /*offset*/){}
    
private:
    void                   on_provider_destroy();
    double                 len_for_cmd() const;

    // msg dispatching
private:
    void on_settings            (aircraft::settings_t const&          m);
    void on_fpl                 (optional<uint32_t> const&  id);

    void on_malfunction         (msg::malfunction_msg   const&  m);
    void on_contact_effect      (msg::contact_effect    const& eff)     ;
    void on_wheel_contact_effect(msg::wheel_contact_effect const& eff)  ;
    
    void on_traj_assign         (msg::traj_assign_msg const &m)         ;

protected:
    void set_fpl_initial_position();
    //void update_atc_state();
    
    nodes_management::manager_ptr  get_nodes_manager() const { return nodes_manager_; }
   


private:
    optional<double> last_len_calc_;

private:

    nodes_management::manager_ptr  nodes_manager_;


private:
    scoped_connection              fms_changed_connection_ ;
    scoped_connection              state_changed_connection_ ;
    scoped_connection              plan_changed_connection_ ;

protected:
    transform_4                    tow_point_transform_;

 
	optional<double>               proc_len_;
    optional<double>               prediction_len_;

    void update_len(double time);


protected:
    //////////////////////////////////////
    fms::trajectory_ptr            traj_;
    /////////////////////////////////////

private:
#if 0 
    tp_sys::control_ptr            tp_sys_;
    fms::tp::aircraft_ptr          airc_tp_;
#endif

    size_t                         providers_count_;
    double                         max_pred_len_;

private:
    boost::optional<unsigned>      old_responder_code_ ;

private:
    scoped_connection              fpl_changed_conn_;

private:
    std::vector< std::pair<double, std::string> > oldi_sequence_ ;


};

} // aircraft
