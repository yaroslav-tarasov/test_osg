#pragma once

#include "application/panels_fwd.h"
#include "application/widget.h"

namespace app
{

struct time_panel
    : widget
{
    
    virtual void set_visible(bool visible) = 0;
    virtual bool visible() = 0; 
    
    virtual void set_time(double time) = 0;
};

BASAPPLIC_API time_panel_ptr create_time_panel();

} // end of namespace app
