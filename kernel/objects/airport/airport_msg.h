
#pragma once 

namespace airport
{
namespace msg 
{

enum id 
{
    mt_settings,
    ap_point_of_view,
    ap_update_point_of_view
};


typedef gen_msg<ap_point_of_view, uint32_t>        changed_pov_msg_t;
typedef gen_msg<ap_update_point_of_view, uint32_t> update_pov_msg_t;

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


} // msg
} // namespace airport

