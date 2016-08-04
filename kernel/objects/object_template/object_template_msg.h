#pragma once

#include "common/arresting_gear.h"

#include "network/msg_base.h"
#include "objects/arresting_gear.h"

namespace arresting_gear
{

using network::gen_msg;

namespace msg 
{


enum id
{          
    ar_settings      ,
    ar_phys_pos      ,
    ar_contact_effect,
    ar_ropes_state   ,
    ar_set_target    ,
};

typedef gen_msg<ar_settings,     arresting_gear::settings_t>    settings_msg;
typedef gen_msg<ar_set_target, boost::optional<uint32_t>>       target_msg;

//! сообщение "физическая позиция ВС"
struct phys_pos_msg
    : network::msg_id<ar_phys_pos>
{
    phys_pos_msg() {}

    phys_pos_msg(geo_point_3 pos, double course)
        : pos(pos), course(course)
    {}

    geo_point_3 pos;
    double course;
};


struct contact_effect
    : network::msg_id<ar_contact_effect>
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

struct ropes_state
    : network::msg_id<ar_ropes_state>
{
    ropes_state() {}

    ropes_state(std::vector<rope_state_t>&& state, double time)
        : state(move(state)), time(time)
    {}

    std::vector<rope_state_t> state;
    double time;
};

REFL_STRUCT(phys_pos_msg)
    REFL_ENTRY(pos)
    REFL_ENTRY(course)
REFL_END()


REFL_STRUCT(contact_effect)
    REFL_ENTRY(contacts)
    REFL_ENTRY(time)
REFL_END()

REFL_STRUCT(ropes_state)
    REFL_ENTRY(state)
    REFL_ENTRY(time)
REFL_END()

} // msg
} // aircraft
