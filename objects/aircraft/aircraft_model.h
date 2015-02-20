#pragma once

#include "network/msg_dispatcher.h"

using network::gen_msg;  // FIXME
using network::msg_t;

#include "aircraft_msg.h"

namespace aircraft
{
    struct model_base
    {
        virtual ~model_base() {};
        void set(msg_t const& msg, bool sure = true)
        {
        }
    };

    struct model
        : info
        , control
        , int_control
        , model_base
    {

    public:
        static const   int                     max_desired_velocity = 20;
        static const   int                     min_desired_velocity = 5;
        inline static  double                  min_radius() {return 18.75;} 
        inline static  double                  step()       {return 2.0;} 



        model( nodes_management::manager_ptr        nodes_manager,
               aircraft::phys_aircraft_ptr          aircraft,
               aircraft::shassis_support_ptr        shassis       )
            : phys_aircraft_(aircraft)
            , shassis_ (shassis)
            , desired_velocity_(min_desired_velocity)
            , nodes_manager_(nodes_manager)
        {
            // from view
 #pragma region view
           if (nodes_manager_)
            {
                //if (!settings_.kind.empty() && 
                //    nodes_manager_->get_model() != get_model(settings_.kind) &&
                //    !has_assigned_fpl())
                //{
                //    nodes_manager_->set_model(get_model(settings_.kind));
                //}

                if (auto tow_point_node = nodes_manager_->find_node("tow_point"))
                    tow_point_transform_ = get_relative_transform(nodes_manager_, tow_point_node);
            }
#pragma endregion

        }

        void check_wheel_brake()
        {
            if (!phys_aircraft_)
                return;

            shassis_->visit_groups([this](aircraft::shassis_group_t & shassis_group)
            {
                if (true || shassis_group.opened && shassis_group.malfunction && !shassis_group.broken)
                {
                    bool has_contact = shassis_group.check_contact(this->phys_aircraft_);
                    if (has_contact)
                        shassis_group.broke(this->phys_aircraft_);
                }
            });
        }

        void on_malfunction_changed( malfunction_kind_t kind ); 

        void update_contact_effects(double time);

        void update(double dt);


        // info
protected:
        cg::geo_point_3 const&  pos                () const override;
        cg::point_3             dpos               () const override;
        cg::cpr                 orien              () const override;
        //settings_t const &  settings           () const override;
        //fpl::info_ptr       get_fpl            () const override;
        //bool                has_assigned_fpl   () const override;
        cg::transform_4 const&  tow_point_transform() const override;

        nodes_management::node_info_ptr root() const override;

        bool malfunction(malfunction_kind_t kind) const override;

        //tp_provider_ptr get_tp_provider(double duration_sec) override;

        //atc_controls_t const&     get_atc_controls() const override;
        //ipo_controls_t const&     get_ipo_controls() const override;
        //aircraft_gui::control_ptr get_gui()          const override;

        //optional<double> get_prediction_length() const override;
        //optional<double> get_proc_length() const override;



   public:
        inline aircraft::shassis_support_ptr get_chassis() {return shassis_;};
        decart_position get_local_position() {return phys_aircraft_->get_local_position();};
        
        void set_trajectory(fms::trajectory_ptr  traj){traj_ = traj;}
        fms::trajectory_ptr  get_trajectory(){return traj_;}               

    private:
        aircraft::phys_aircraft_ptr            phys_aircraft_;
        aircraft::shassis_support_ptr          shassis_;
        double                                 desired_velocity_;
        fms::trajectory_ptr                    traj_;

        // from view
#pragma region view
    private:
        std::array<bool, MF_SIZE>    malfunctions_;
    private:
        //conflicts_manager::control_ptr conflicts_manager_;
        nodes_management::manager_ptr  nodes_manager_;
        //aircraft_fms::info_ptr         fms_info_;
protected:
        cg::transform_4 tow_point_transform_;
#pragma endregion 

    };


}