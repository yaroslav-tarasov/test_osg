#pragma once
#include "network/msg_base.h"


namespace flock
{

namespace manager
{

struct settings_t
{
    settings_t()
        : _childAmount(250)
    {}

	int _childAmount;

};



namespace msg
{

enum msg_type
{
    mt_settings
};

struct settings_msg
    : network::msg_id<mt_settings>
{
    settings_t settings;

    settings_msg(settings_t const& settings)
        : settings(settings)
    {
    }
    
    settings_msg()
    {
    }
};

REFL_STRUCT(settings_msg)
    REFL_ENTRY(settings)
REFL_END()
} // messages 



REFL_STRUCT(settings_t)
    REFL_ENTRY(_childAmount)
REFL_END()


} // manager


} // namespace flock



