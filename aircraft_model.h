#pragma once

namespace aircraft
{

    struct model
        : info
        , control
    {

    public:
        static const   int                     max_desired_velocity = 20;
        static const   int                     min_desired_velocity = 5;
        inline static  double                  min_radius() {return 18.75;} 
        inline static  double                  step()       {return 2.0;} 



        model( aircraft::phys_aircraft_ptr          aircraft,
               aircraft::shassis_support_ptr          shassis)
            : phys_aircraft_(aircraft)
            , shassis_ (shassis)
            , desired_velocity_(min_desired_velocity)
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

        void on_malfunction_changed( malfunction_kind_t kind ) 
        {
            if (kind == MF_CHASSIS_FRONT)
            {
                shassis_->set_malfunction(SG_FRONT, malfunction(kind));
            }
            else if (kind == MF_CHASSIS_REAR_LEFT)
            {
                shassis_->set_malfunction(SG_REAR_LEFT, malfunction(kind));
            }
            else if (kind == MF_CHASSIS_REAR_RIGHT)
            {
                shassis_->set_malfunction(SG_REAR_RIGHT, malfunction(kind));
            }
            else if (kind == MF_ONE_ENGINE || kind == MF_ALL_ENGINES)
            {
                double factor = 1;
                if (malfunction(MF_ALL_ENGINES))
                    factor = 0;
                else if (malfunction(MF_ONE_ENGINE))
                    factor = 0.7;

                // FIXME TODO OR NOT TODO
                //auto controls = flight_manager_control_->get_controls();
                //controls.engine_factor = factor;
                //flight_manager_control_->set_controls(controls);
            }
            else if (kind == MF_FUEL_LEAKING)
            {   
                // FIXME TODO OR NOT TODO
                //auto controls = flight_manager_control_->get_controls();
                //controls.fuel_leaking = true;
                //flight_manager_control_->set_controls(controls);
            }
        }

        void update(double dt)
        {
            auto it = this;

            if((*it).traj_.get())
            {
                if ((*it).traj_->cur_len() < (*it).traj_->length())
                {
                    (*it).phys_aircraft_->set_prediction(15.); 
                    (*it).phys_aircraft_->freeze(false);
                    const double  cur_len = (*it).traj_->cur_len();
                    (*it).traj_->set_cur_len ((*it).traj_->cur_len() + dt*(*it).desired_velocity_);
                    const double  tar_len = (*it).traj_->cur_len();
                    decart_position target_pos;

                    target_pos.pos = cg::point_3((*it).traj_->kp_value(tar_len),0);
                    geo_position gtp(target_pos, get_base());
                    (*it).phys_aircraft_->go_to_pos(gtp.pos ,gtp.orien);


                    const double curs_change = (*it).traj_->curs_value(tar_len) - (*it).traj_->curs_value(cur_len);

                    if(cg::eq(curs_change,0.0))
                        (*it).desired_velocity_ = aircraft::model::max_desired_velocity;
                    else
                        (*it).desired_velocity_ = aircraft::model::min_desired_velocity;

                    // const decart_position cur_pos = _phys_aircrafts[0].phys_aircraft_->get_local_position();

                    //std::stringstream cstr;

                    //cstr << std::setprecision(8) 
                    //     << "curr_pods_len:  "             << (*it).traj->cur_len() 
                    //     << "    desired_velocity :  "     << (*it).desired_velocity_   
                    //     << "    delta curs :  "  << curs_change
                    //     << ";   cur_pos x= "     << cur_pos.pos.x << " y= "  << cur_pos.pos.y  
                    //     << "    target_pos x= "  << target_pos.pos.x << " y= "  << target_pos.pos.y <<"\n" ;

                    //OutputDebugString(cstr.str().c_str());
                }
                else
                {

                    cg::point_3 cur_pos = phys_aircraft_->get_local_position().pos;
                    cg::point_3 d_pos = phys_aircraft_->get_local_position().dpos;
                    cg::point_3 trg_p((*it).traj_->kp_value((*it).traj_->length()),0);
                    d_pos.z = 0;
                    if(cg::distance(trg_p,cur_pos) > 1.0 && cg::norm(d_pos) > 0.05)
                    {   
                        decart_position target_pos;
                        target_pos.pos = trg_p;
                        geo_position gp(target_pos, get_base());
                        (*it).phys_aircraft_->go_to_pos(gp.pos ,gp.orien);
                    }
                    else
                    {
                        // (*it).traj.reset();
                        (*it).phys_aircraft_->freeze(true);
                    }
                }

            }

            phys_aircraft_->update();

        }

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
        inline decart_position get_local_position() {return phys_aircraft_->get_local_position();};
        inline void set_trajectory(fms::trajectory_ptr  traj){traj_ = traj;}
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