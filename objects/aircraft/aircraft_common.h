#pragma once

#include "aircraft/aircraft_msg.h"
#include "phys/aircraft.h"

namespace aircraft
{
    struct phys_aircraft
    {
        virtual void                 update() = 0;
        virtual void                 attach_tow(bool attached) = 0;
        virtual void                 go_to_pos(cg::geo_point_3 const& pos, cg::quaternion const& orien)  = 0;
        virtual geo_position         get_position() const = 0;
        virtual decart_position      get_local_position() const = 0;
        virtual void                 set_air_cfg(fms::air_config_t cfg) = 0;
        virtual void                 set_prediction(double prediction) = 0;
        virtual geo_position         get_wheel_position( size_t i ) const = 0;
        virtual phys::rigid_body_ptr get_rigid_body() const = 0;
        virtual void                 set_steer   (double steer) = 0;
        virtual double               get_steer () = 0;
        virtual std::vector<phys::aircraft::contact_info_t> get_body_contacts() const = 0;
        virtual bool                 has_wheel_contact(size_t id) const = 0;
        virtual double               wheel_skid_info(size_t id) const = 0;
        virtual void                 remove_wheel(size_t id) = 0;
        virtual size_t               get_zone() const = 0;
        virtual void                 set_malfunction(bool malfunction) = 0;
        virtual void                 freeze(bool freeze) = 0;
        virtual ~phys_aircraft() {}
    };


    typedef polymorph_ptr<phys_aircraft> phys_aircraft_ptr;


    struct shassis_t
    {
        shassis_t(nm::node_control_ptr node, nodes_management::node_control_ptr wheel_node, double radius)
            : node(node)
            , wheel_node(wheel_node)
            , radius(radius)
            , landing_dust(false)
        {}

        nodes_management::node_control_ptr node;
        nodes_management::node_control_ptr wheel_node;
        std::vector<size_t>                phys_wheels;
        double radius;
        bool landing_dust;

        void clear_wheels()
        {
            phys_wheels.clear();
        }

        void freeze()
        {
#if 0   // TODO or not TODO
            {
                auto pos = wheel_node->position();
                pos.local().dpos = point_3();
                pos.local().omega = point_3();
                wheel_node->set_position(pos);
            }
            {
                auto pos = node->position();
                pos.local().dpos = point_3();
                pos.local().omega = point_3();
                node->set_position(pos);
            }
#endif
        }

    };

    struct shassis_group_t
    {
        shassis_group_t(nm::node_control_ptr node, bool is_front)
            : node(node)
            , malfunction(false)
            , opened(true/*false*/) // FIXME TODO а у нас выпущены
            , broken(false)
            , is_front(is_front)
        {

        }

        bool malfunction;
        bool opened;
        bool broken;
        bool is_front;
        nm::node_control_ptr node;

        std::vector<shassis_t> shassis;

        void clear_wheels()
        {
            for (auto it= shassis.begin(); it != shassis.end(); ++it)
                it->clear_wheels();
        }

        void freeze()
        {
            for (auto it= shassis.begin(); it != shassis.end(); ++it)
                it->freeze();
        }

        void open(bool immediate = false)
        {
            if (immediate)
                node->play_animation("clip1", 5, 0., 0.);
            else
                node->play_animation("clip1", 5, 1., -1.);


            opened = true;
        }

        void close(bool immediate = false)
        {
            if (immediate)
                node->play_animation("clip1", 5, 1., 0.);
            else
                node->play_animation("clip1", 5, 0., 1.);


            opened = false;
        }

        bool check_contact(phys_aircraft_ptr aircraft_phys)
        {
            bool has_contact = false;
            for (auto it= shassis.begin(); it != shassis.end(); ++it)
            {
                for (size_t j = 0; j < it->phys_wheels.size(); ++j)
                {
                    if (aircraft_phys->has_wheel_contact(it->phys_wheels[j]))
                    {
                        has_contact = true;
                        break;
                    }
                }

            }

            return has_contact;
        }

        void broke(phys_aircraft_ptr aircraft_phys)
        {
            broken = true;
            for (auto it= shassis.begin(); it != shassis.end(); ++it)
            {
                it->node->set_visibility(false);
                it->freeze();
                for (size_t j = 0; j < it->phys_wheels.size(); ++j)
                    aircraft_phys->remove_wheel(it->phys_wheels[j]);

                it->phys_wheels.clear();
            }
        }

        void add_chassis(shassis_t const& s)
        {
            shassis.push_back(s);
        }

    };


    enum shasis_group
    {
        SG_FRONT = 0, SG_REAR_LEFT, SG_REAR_RIGHT
    };

    struct shassis_support
    {
        virtual void visit_groups   (std::function<void(shassis_group_t &)> out) = 0;
        virtual void visit_chassis  (std::function<void(shassis_group_t const&, shassis_t&)> out) = 0;
        virtual void set_malfunction(shasis_group group, bool val) = 0;
        virtual void freeze() = 0;

        virtual ~shassis_support() {}
    };

    typedef polymorph_ptr<shassis_support> shassis_support_ptr;


} // aircraft
