#pragma once

#include "human_fwd.h"

#include "phys/phys_sys_fwd.h"
#include "atc/position.h"


namespace human
{

    struct model_info
    {
        virtual ~model_info() {}

        // virtual phys::rigid_body_ptr get_rigid_body() const = 0;
        // phys::ray_cast_human::info_ptr
        virtual bool                 tow_attached  () const = 0;
        virtual cg::point_3          tow_offset    () const = 0;
        virtual geo_position         get_phys_pos  () const = 0;
    };

    struct model_control
    {
        virtual ~model_control() {}

        virtual void                 set_tow_attached( optional<uint32_t> attached, boost::function<void()> tow_invalid_callback ) = 0;
        virtual void                 set_steer       ( double steer ) = 0;
        virtual void                 set_brake       ( double brake ) = 0;
        virtual void                 set_desired     ( double time,const cg::point_3& pos, const cg::quaternion& q, const double speed )     = 0;
        virtual void                 set_ext_wind    ( double speed, double azimuth ) =0;
    };

} // end of human
