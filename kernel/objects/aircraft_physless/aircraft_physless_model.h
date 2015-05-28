#pragma once

#include "aircraft_physless_view.h"
//#include "sync_fsm/sync_fsm.h"

#include "network/msg_dispatcher.h"

using network::gen_msg;  // FIXME
using network::msg_t;

#include "aircraft_physless_msg.h"
#include "common\airports_manager.h"

namespace aircraft_physless
{
    namespace ums
    {

        struct state_t
        {
            state_t()
                : course(0)
                , fuel_mass  (1)
                , TAS   (0)
                //, cfg   (CFG_CR)
            {}

            state_t(cg::geo_point_3 const& pos, double course, double fuel_mass, double TAS/*, air_config_t cfg*/)
                : pos   (pos)
                , course(course)
                , fuel_mass (fuel_mass)
                , TAS   (TAS)
                //, cfg   (cfg)
            {}

            cg::geo_point_3 pos;
            double course;
            double fuel_mass;
            double TAS;
            //air_config_t cfg;
        };

        struct pilot_state_t
        {
            pilot_state_t(state_t const& dyn_state)
                : dyn_state(dyn_state)
            {}

            pilot_state_t()
            {}

            state_t                 dyn_state;
        };
    }

    struct state_t : ums::pilot_state_t
    {
        state_t();
        state_t(ums::pilot_state_t const& ps, double pitch, double roll, uint32_t version);
        cpr orien() const;

        double pitch;
        double roll;
    };

    inline state_t::state_t()
        : pitch(0)
        , roll(0)
    {}

    inline state_t::state_t(ums::pilot_state_t const& ps, double pitch, double roll, uint32_t version)
        : pilot_state_t(ps)
        , pitch(pitch)
        , roll(roll)
    {}

    inline cpr state_t::orien() const
    {
        return cpr(dyn_state.course, pitch, roll);
    }

    struct model
        : view
        , aircraft::model_info                // интерфейс информации о модели
        , aircraft::model_control             // интерфейс управления моделью
        , aircraft::int_control
        //, sync_fsm::self_t
    {
        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:

        model( kernel::object_create_t const& oc, dict_copt dict);

    public:
        void on_malfunction_changed( aircraft::malfunction_kind_t kind ) override; 
        void on_new_contact_effect      (double /*time*/, std::vector<contact_t> const& /*contacts*/) override;

    // base_presentation
        FIXME(private)
        void update(double dt);

        // base_view_presentation
    protected:
        void on_child_removing(kernel::object_info_ptr child) override ;
        void on_object_destroying(object_info_ptr object)     override ;

        // model_info
    private:
       // phys::rigid_body_ptr get_rigid_body() const;
        phys::rigid_body_ptr model::get_rigid_body() const
        {
            return  phys::rigid_body_ptr();
        }
        point_3              tow_offset    () const;
        bool                 tow_attached  () const;
        geo_position         get_phys_pos  () const;
        double               rotors_angular_speed () const override;

        // model_control
    private:
        void                 set_tow_attached(optional<uint32_t> attached, boost::function<void()> tow_invalid_callback);
        void                 set_steer                ( double steer ) override;
        void                 set_brake                ( double brake ) override;
        void                 set_rotors_angular_speed ( double val ) override;

        // sync_fsm::self_t
    private:  
        geo_position           fms_pos() const;
        airports_manager::info_ptr get_airports_manager() const;
        nodes_management::manager_ptr get_nodes_manager() const;
        shassis_support_ptr    get_shassis() const;
        rotors_support_ptr     get_rotors() const;

        ::fms::trajectory_ptr    get_trajectory() const;

        geo_position           get_root_pos() const;
        bool is_fast_session() const;
        void set_desired_nm_pos  (geo_point_3 const& pos);
        void set_desired_nm_orien(quaternion const& orien);
        //void switch_sync_state(sync_fsm::state_ptr state);
        void freeze_position();
        void set_phys_aircraft(phys_aircraft_ptr phys_aircraft);
        void set_nm_angular_smooth(double val);
    private:
        void update_model();
        void sync_phys(double dt);
        void calc_phys_controls(double & slide_angle, double & thrust, double & attack_angle, double q, cg::rotation_3 const& vel_rotation, point_3 const& desired_accel, point_3 const& wind, bool reverse, bool low_attack);
        void sync_fms(bool force = false);
        bool create_phys_aircraft_impl();  // а тут вообще пусто
        void init_shassi_anim ();
        void update_shassi_anim (double time);
        void update_contact_effects(double time);
        void check_wheel_brake();
        void check_rotors_malfunction();
        void on_time_factor_changed(double time, double factor);
            
        void sync_nm_root(double dt);

   public:
        inline aircraft::shassis_support_ptr get_chassis() {return shassis_;};
        decart_position get_local_position() {return decart_position()/*phys_aircraft_->get_local_position()*/;};
        
        void set_trajectory(fms::trajectory_ptr  traj){traj_ = traj;}
        fms::trajectory_ptr  get_trajectory(){return traj_;} 

    private:
        model_system *                         sys_;
        optional<double>                       last_update_;

        airports_manager::info_ptr             airports_manager_;
        
        nodes_management::node_control_ptr     root_;
        nodes_management::node_control_ptr     elev_rudder_node_;
        nodes_management::node_control_ptr     rudder_node_;
        nodes_management::node_info_ptr        tow_point_node_;
        nodes_management::node_info_ptr        body_node_;

        rotors_support_ptr                 rotors_;
        shassis_support_ptr                shassis_;
        optional<double>                   last_shassi_play_;
        bool                               shassi_anim_inited_;

        double                                 rotors_angular_speed_;

        //aircraft::phys_aircraft_ptr            phys_aircraft_;

        optional<geo_point_3>                  desired_nm_pos_;
        optional<quaternion>                   desired_nm_orien_;
        double                                 nm_ang_smooth_;
        
        boost::function<void()>                tow_invalid_callback_;
        optional<uint32_t>                     tow_attached_;

    private:
        bool fast_session_;

    //private:
    //    sync_fsm::state_ptr sync_state_;
        state_t                                _state;
        inline        state_t  const&                get_state() const {return _state;}
    FIXME(Офигенная статическая переменная)
public:
    static const double shassi_height_;

    };


}