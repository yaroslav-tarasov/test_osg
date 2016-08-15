#pragma once

#include "common/cloud_zone.h"

namespace cloud_zone
{
namespace msg 
{
//! сообщение зоны облачности
enum
{
    em_settings = 0,
} ;
//! тело сообщения зоны облачности
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
} ;


REFL_STRUCT(settings_msg)
    REFL_ENTRY(settings)
REFL_END   ()

} // end of msg
} // end of cloud_zone