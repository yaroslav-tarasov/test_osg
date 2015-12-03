#pragma once

#include "application/panels/time_panel.h"


class time_panel_impl
        : public app::time_panel
{
    
public:
    time_panel_impl ( );
    ~time_panel_impl();

    void set_visible(bool visible) override;
    bool visible    ()             override;

    void set_time   (double ms)  override;
};
