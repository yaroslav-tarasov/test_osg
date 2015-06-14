#pragma once

#include "application/panels/vis_settings_panel.h"


class vis_settings_panel_impl
        : public app::vis_settings_panel
{
    
public:
    vis_settings_panel_impl (  const app::zones_t &zones );
    ~vis_settings_panel_impl();

    void set_visible(bool visible) override;
    bool visible    () override;
private:
	void init_menu_bar(CEGUI::Menubar* menuBar);
};
