#pragma once

#include "common/environment.h"

namespace environment
{
namespace msg 
{
//! сообщения "окружения"
enum
{
    em_settings = 0,
    em_start_time,
};

//! сообщение - настройки 
struct settings_msg 
    : network::msg_id<em_settings>
{
    settings_t settings;

    settings_msg()
    {
    }

    settings_msg(settings_t const& st)
        : settings(st)
    {
    }
};

//! сообщение - время начала
struct start_time
    : network::msg_id<em_start_time>
{
    start_time(double time = 0.)
        : time(time)
    {
    }

    double time;
};


REFL_STRUCT(settings_msg)
    REFL_ENTRY(settings)
REFL_END   ()

REFL_STRUCT(start_time)
    REFL_ENTRY(time)
REFL_END   () 

} // msg
} // environment