#pragma once
#include "network/msg_base.h"


namespace flock
{

namespace child
{

struct settings_t
{
    settings_t()
        : icao_code("UUEE")
    {}

    string   icao_code;
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


inline string get_model(string icao_code)
{
    if (icao_code == "UUEE")
        return "sheremetyevo";
    else if (icao_code == "URSS")
        return "adler";
    else if (icao_code == "UMMS" || icao_code == "UMMM")
        return "minsk";

    return "";
}

inline bool valid_icao(string icao_code)
{
    return icao_code == "UUEE" || 
           icao_code == "URSS" ||
           icao_code == "UHWW" ||
           icao_code == "UMMS" ||
           icao_code == "UMMM"
           ;     
}


REFL_STRUCT(settings_t)
    REFL_ENTRY(icao_code)
REFL_END()


} // child


} // namespace flock



