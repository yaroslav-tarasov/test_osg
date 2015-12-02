#include "stdafx.h"

#pragma optimize( "", off )

// Ну и куда то убрать надо 
FIXME(Заголовок как в рефренсном)
#include <cegui/CEGUI.h>

#include "time_panel_impl.h"

using namespace CEGUI;

#include "cegui_utils.h"

time_panel_impl::time_panel_impl()
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    
	WindowManager& winMgr = WindowManager::getSingleton();
    
    PushButton* TimeButtonOK = static_cast<PushButton*>(
        WindowManager::getSingleton().createWindow("TaharezLook/Button", "TimeButtonOK") );
    TimeButtonOK->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim(0.75f)));
    TimeButtonOK->setSize( USize(cegui_reldim(0.4f), cegui_reldim(0.15f)) );
    TimeButtonOK->setText( "OK" );
}

time_panel_impl::~time_panel_impl()
{
}

void time_panel_impl::set_time(double time)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
}

void time_panel_impl::set_visible(bool visible)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    //root->getChild(combo_name)->setVisible(visible);
    //root->getChild(btn_exit_name)->setVisible(visible);
    //root->getChild(main_menu_name)->setVisible(visible);

}

bool time_panel_impl::visible()
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    // return root->getChild(btn_exit_name)->isVisible() || root->getChild(combo_name)->isVisible();
    return true;
}


namespace app
{
    time_panel_ptr create_time_panel()
    {
        return boost::make_shared<time_panel_impl>();
    }
}
