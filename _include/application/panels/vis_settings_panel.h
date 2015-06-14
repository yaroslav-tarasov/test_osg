#pragma once

//#include "application/application_fwd.h"
#include "application/panels_fwd.h"
#include "application/widget.h"
//#include "common/event.h"

namespace app
{

typedef  std::vector< std::pair<int,std::wstring> > zones_t;

struct vis_settings_panel
    : widget
{
    
    virtual void set_visible(bool visible) = 0;
    virtual bool visible() = 0; 

    DECLARE_EVENT(zone_changed , (int) );
    DECLARE_EVENT(exit_app     , ()    );
	DECLARE_EVENT(set_lights   , (bool) );
};

/*BASAPPLIC_API*/ vis_settings_panel_ptr create_vis_settings_panel( const zones_t &zones);

} // end of namespace app
