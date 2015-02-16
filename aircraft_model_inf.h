#pragma once

namespace aircraft
{
    enum malfunction_kind_t
    {
        MF_CHASSIS_FRONT = 0,
        MF_CHASSIS_REAR_LEFT,
        MF_CHASSIS_REAR_RIGHT,
        MF_FIRE_ON_BOARD,
        MF_SMOKE_ON_BOARD,
        MF_FUEL_LEAKING,
        MF_ONE_ENGINE,
        MF_ALL_ENGINES, 

        MF_SIZE
    };

    struct info
    {
        virtual ~info(){}

        virtual cg::geo_point_3 const&  pos     () const = 0;
        virtual cg::point_3             dpos    () const = 0;
        virtual cg::cpr                 orien   () const = 0;
        //virtual settings_t const &  settings() const = 0;
        //virtual fpl::info_ptr       get_fpl () const = 0;
        //virtual bool                has_assigned_fpl() const = 0;

        virtual cg::transform_4 const&  tow_point_transform() const = 0;

        virtual nodes_management::node_info_ptr root() const = 0;
        virtual bool malfunction(malfunction_kind_t kind) const = 0;

        //virtual tp_provider_ptr  get_tp_provider(double duration_sec) = 0;

        //virtual aircraft_fms::info_ptr get_fms() const = 0;


        //virtual atc_controls_t const& get_atc_controls() const = 0;
        //virtual ipo_controls_t const& get_ipo_controls() const = 0;

        //virtual aircraft_gui::control_ptr get_gui() const = 0;

        //virtual optional<double> get_prediction_length() const = 0;
        //virtual optional<double> get_proc_length() const = 0;

        //DECLARE_EVENT(assigned_fpl_changed, (fpl::info_ptr));
        //DECLARE_EVENT(responder_changed, (fpl::info*));
    };

    struct control
    {
        virtual ~control(){}

        //virtual void assign_fpl  (fpl::info_ptr fpl_obj) = 0;
        //virtual void unassign_fpl() = 0;

        //virtual void set_kind(std::string const& kind) = 0;
        //virtual void set_turbulence(unsigned turb) = 0;

        //virtual void set_atc_controls(atc_controls_t const& controls) = 0 ;
        //virtual void set_ipo_controls(ipo_controls_t const& controls) = 0 ;
    };

    struct int_control
    {
         virtual ~int_control(){}
         virtual void update(double dt)=0;
         virtual void set_trajectory(fms::trajectory_ptr  traj)=0;
         virtual fms::trajectory_ptr  get_trajectory()=0;
         virtual decart_position get_local_position()=0; 
    };

}