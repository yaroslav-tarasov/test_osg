#pragma once

#include "helicopter_physless/helicopter_physless_msg.h"
#include "phys/aircraft.h"
#include "aircraft/aircraft_common.h"


namespace helicopter_physless
{
    enum rotor_state_t : uint16_t
    {
        rs_static,
        rs_dynamic,
        rs_sagged
    };

#if 0
    struct phys_aircraft
    {
        virtual void                 update            () = 0;
        virtual void                 attach_tow        ( bool attached ) = 0;
        virtual void                 go_to_pos         ( cg::geo_point_3 const& pos, cg::quaternion const& orien )  = 0;
        virtual geo_position         get_position      () const = 0;
        virtual decart_position      get_local_position() const = 0;
        virtual void                 set_air_cfg       ( fms::air_config_t cfg) = 0;
        virtual void                 set_prediction    ( double prediction ) = 0;
        virtual geo_position         get_wheel_position( size_t i ) const = 0;
        virtual phys::rigid_body_ptr get_rigid_body    () const = 0;
        virtual void                 set_steer         ( double steer ) = 0;
        virtual void                 set_brake         ( double brake ) = 0;
        virtual double               get_steer         () = 0;
        virtual std::vector<phys::aircraft::contact_info_t> get_body_contacts() const = 0;
        virtual bool                 has_wheel_contact ( size_t id ) const = 0;
        virtual double               wheel_skid_info   ( size_t id ) const = 0;
        virtual void                 remove_wheel      ( size_t id ) = 0;
        virtual size_t               get_zone          () const = 0;
        virtual void                 set_malfunction   ( bool malfunction ) = 0;
        virtual void                 freeze            ( bool freeze ) = 0;
        virtual ~phys_model() {}
    };


    typedef polymorph_ptr<phys_aircraft> phys_aircraft_ptr;
#endif

} // aircraft
