#pragma once

#include "common/aircraft.h"

#include "network/msg_base.h"
#include "objects/aircraft_atc.h"
#include "objects/aircraft.h"
#include "fms/trajectory.h"

namespace aircraft_physless
{

using network::gen_msg;

namespace msg 
{
//! сообщения, относящиеся к ВС
enum id
{          
    am_settings      ,
    am_malfunction   ,
    am_contact_effect,
    am_wheel_contact_effect,
    am_traj_assign   ,
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



} // aircraft
