#pragma once

#include "common/aircraft.h"
//#include "common/helicopter_physless.h"

#include "network/msg_base.h"
#include "objects/aircraft_atc.h"
#include "objects/aircraft.h"
//#include "objects/helicopter_physless.h"
#include "fms/trajectory.h"

#include "helicopter_physless_state.h"

namespace helicopter_physless
{

using network::gen_msg;

namespace msg 
{
//! сообщения, относящиеся к ВС
enum id
{          
    am_settings      ,
    am_malfunction   ,
    am_engines_state ,
    am_contact_effect,
    am_wheel_contact_effect,
    am_traj_assign   , 
    afm_state        ,
    afm_kind,
    afm_local_meteo,

};

//! тела сообщений, сгенерированные специальным шаблоном
typedef gen_msg<am_settings,     aircraft::settings_t>         settings_msg;

//! сообщение "особый случай" (повреждение) ВС
struct malfunction_msg
    : network::msg_id<am_malfunction>
{
    malfunction_msg() {}

    malfunction_msg(aircraft::malfunction_kind_t kind, bool enabled)
        : kind(kind), enabled(enabled)
    {}

    aircraft::malfunction_kind_t kind;
    bool enabled;
};

struct engine_state_msg
    : network::msg_id<am_engines_state>
{
    engine_state_msg() {}

    engine_state_msg( aircraft::engine_state_t state)
        : state(state)
    {}
    aircraft::engine_state_t      state;
};

REFL_STRUCT(engine_state_msg)
    REFL_ENTRY(state)
REFL_END()

//! сообщение 
struct traj_assign_msg
    : network::msg_id<am_traj_assign>
{
    traj_assign_msg() {}

    traj_assign_msg(const fms::traj_data& traj)
        : traj(traj)
    {}

    traj_assign_msg(const fms::traj_data&& traj)
        : traj(move(traj))
    {}

     fms::traj_data traj;
};

//! сообщение 
struct wheel_contact_effect
    : network::msg_id<am_wheel_contact_effect>
{
    wheel_contact_effect() {}

    wheel_contact_effect(double time, point_3f  const& offset, point_3f  const& vel)
        : time(time), offset(offset), vel(vel)
    {}

    point_3f vel;
    point_3f offset;
    double time;
};

struct contact_effect
    : network::msg_id<am_contact_effect>
{
    struct contact_t
    {
        contact_t(){}
        contact_t(point_3f  const& offset, point_3f  const& vel)
            : offset(offset), vel(vel)
        {}

        point_3f vel;
        point_3f offset;

        REFL_INNER(contact_t)
            REFL_ENTRY(offset)
            REFL_ENTRY(vel)
        REFL_END()
    };

    contact_effect() {}

    contact_effect(std::vector<contact_t>&& contacts, double time)
        : contacts(move(contacts)), time(time)
    {}

    std::vector<contact_t> contacts;
    double time;
};

REFL_STRUCT(traj_assign_msg)
    REFL_ENTRY(traj)
REFL_END()


REFL_STRUCT(malfunction_msg)
    REFL_ENTRY(kind)
    REFL_ENTRY(enabled)
REFL_END()

REFL_STRUCT(contact_effect)
    REFL_ENTRY(contacts)
    REFL_ENTRY(time)
REFL_END()

REFL_STRUCT(wheel_contact_effect)
    REFL_ENTRY(time)
    REFL_ENTRY(vel)
    REFL_ENTRY(offset)
REFL_END()

} // msg

namespace msg 
{

struct state_msg
    : network::msg_id<afm_state>
{               
    state_msg();
    explicit state_msg(state_t const& state);
    operator state_t() const;

    geo_point_2   pos;
    float         height;
    cprf          orien;
    float         TAS;
    float         fuel_mass;
    unsigned char cfg;
    unsigned char alt_state;
    uint32_t      version;
};


typedef network::gen_msg<afm_kind           , string>                       kind_msg;


REFL_STRUCT(state_msg)
    REFL_ENTRY(pos)
    REFL_ENTRY(height)
    REFL_ENTRY(orien)
    REFL_ENTRY(TAS)
    REFL_ENTRY(fuel_mass)
    //REFL_ENTRY(cfg)
    //REFL_ENTRY(alt_state)
    //REFL_ENTRY(version)
REFL_END   ()


    inline state_msg::state_msg()
{
}

inline state_msg::state_msg(state_t const& state)
    : pos(state.dyn_state.pos)
    , height((float)state.dyn_state.pos.height)
    , orien(state.orien())
    , TAS((float)state.dyn_state.TAS)
    //, cfg((unsigned char)state.dyn_state.cfg)
    , fuel_mass((float)state.dyn_state.fuel_mass)
    //, alt_state(state.alt_state)
    //, version(state.version)
{
}

inline state_msg::operator state_t() const
{
    ums::pilot_state_t pilot(ums::state_t(geo_point_3(pos, height), orien.course, fuel_mass, TAS/*, (fms::air_config_t)cfg*/)/*, (fms::alt_state_t)alt_state*/);

    return state_t(pilot, orien.pitch, orien.roll, version);
}

struct local_meteo_msg 
    : network::msg_id<afm_local_meteo>
{
    float wind_speed, wind_azimuth, wind_gusts;

    local_meteo_msg() 
        : wind_speed(0)
        , wind_azimuth(0)
        , wind_gusts(0)
    {}

    local_meteo_msg(float wind_speed, float wind_azimuth, float wind_gusts) 
        : wind_speed  (wind_speed)
        , wind_azimuth(wind_azimuth)
        , wind_gusts  (wind_gusts)
    {}
};

REFL_STRUCT(local_meteo_msg)
    REFL_ENTRY(wind_speed)
    REFL_ENTRY(wind_azimuth)
    REFL_ENTRY(wind_gusts)
REFL_END()


} // msg


} // aircraft
