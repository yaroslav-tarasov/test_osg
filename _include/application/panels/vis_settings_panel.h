#pragma once

#include "application/application_fwd.h"
#include "application/panels_fwd.h"
#include "application/widget.h"
//#include "common/event.h"

namespace app
{

typedef  std::vector< std::pair<int,std::wstring> > zones_t;


struct cloud_params_t
{
    float x;
    float y;
    float radius_x;
    float radius_y;
    float height;
    int   p_type;
    float intensity;

};

struct settings_t
{
    bool        shadow;
    bool        shadow_for_smoke;
    std::array<cloud_params_t,2> clouds; 
    float       intensity;
};

struct vis_settings_panel
    : widget
{
    
    virtual void set_visible(bool visible) = 0;
    virtual bool visible() = 0; 
    
    virtual void set_light(bool on) = 0;
    
    DECLARE_EVENT(zone_changed    , (int) );
    DECLARE_EVENT(exit_app        , ()    );
	DECLARE_EVENT(set_lights      , (bool) );
    DECLARE_EVENT(set_shadows     , (bool) );
    DECLARE_EVENT(set_shadows_part, (bool) );
	DECLARE_EVENT(set_map	      , (float) );
    DECLARE_EVENT(set_cloud_param , (cloud_params_t) );
    DECLARE_EVENT(set_global_intensity , (float) );
};

BASAPPLIC_API vis_settings_panel_ptr create_vis_settings_panel( const zones_t &zones, const app::settings_t& s);

} // end of namespace app
