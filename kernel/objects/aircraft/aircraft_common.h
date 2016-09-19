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
        virtual void                 go_to_pos(geo_position const& pos)  = 0;
        virtual geo_position         get_position() const = 0;
        virtual decart_position      get_local_position() const = 0;
        virtual void                 set_air_cfg(fms::air_config_t cfg) = 0;
        virtual void                 set_prediction(double prediction) = 0;
        virtual geo_position         get_wheel_position( size_t i ) const = 0;
        virtual phys::rigid_body_ptr get_rigid_body() const = 0;
        virtual void                 set_steer   (double steer) = 0;
        virtual void                 set_brake( double brake ) = 0;
        virtual double               get_steer () = 0;
        virtual std::vector<phys::aircraft::contact_info_t> get_body_contacts() const = 0;
        virtual bool                 has_wheel_contact(size_t id) const = 0;
        virtual double               wheel_skid_info(size_t id) const = 0;
        virtual void                 remove_wheel(size_t id) = 0;
        virtual size_t               get_zone() const = 0;
        virtual void                 set_malfunction(bool malfunction) = 0;
        virtual void                 freeze(bool freeze) = 0;
        
        virtual void                 force_pos_setup(bool f) = 0;

        virtual                      ~phys_aircraft() {}
    };

    typedef polymorph_ptr<phys_aircraft> phys_aircraft_ptr;


} // aircraft


