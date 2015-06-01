#include "stdafx.h"
#include "vis_settings_panel_impl.h"

// Ну и куда то убрать надо 
FIXME(Заголовок как в рефренсном)
#include <cegui/CEGUI.h>

using namespace CEGUI;

const  String combo_name    ( "selModeBox" );
const  String btn_exit_name ( "btnExit"    );

vis_settings_panel_impl::vis_settings_panel_impl(  const app::zones_t &zones )
    /*: */
{
    // connect(ui_->close_btn        , SIGNAL(clicked())               , this, SLOT(close_clicked_slot        ()   ));

    GUIContext& context = System::getSingleton().getDefaultGUIContext();

    SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    // context.getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");

    FontManager::getSingleton().createFromFile( "DejaVuSans-10.font" );
    context.setDefaultFont( "DejaVuSans-10" );
    context.getDefaultFont()->setAutoScaled(CEGUI::ASM_Disabled);

    Window* root = WindowManager::getSingleton().createWindow( "DefaultWindow", "Root" );
    context.setRootWindow(root);

    PushButton* btnExit = static_cast<PushButton*>(
        WindowManager::getSingleton().createWindow("TaharezLook/Button", btn_exit_name) );

    root->addChild(btnExit);
    btnExit->setPosition(UVector2(cegui_reldim(0.95f), cegui_reldim(0.01f)));
    btnExit->setSize( USize(cegui_reldim(0.04f), cegui_reldim(0.02f)) );
    btnExit->setText( "Exit" );
    
    btnExit->subscribeEvent(PushButton::EventClicked, 
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
        {

            exit_app_signal_(); 
            return true;

        })
        ); 

    

    Combobox* cbbo = static_cast<Combobox*>( CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox", combo_name));
    root->addChild(cbbo);
    cbbo->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.01f)));
    //cbbo->setSize(USize(cegui_reldim(0.66f), cegui_reldim( 0.33f)));

    ListboxTextItem* itm;
    CEGUI::Win32StringTranscoder stc;
    const CEGUI::Image* sel_img = &ImageManager::getSingleton().get("TaharezLook/MultiListSelectionBrush");

    for (auto it = zones.begin(); it!=zones.end();++it)
    {
        itm = new ListboxTextItem(stc.stringFromStdWString(it->second), it->first);
        itm->setSelectionBrushImage(sel_img);
        cbbo->addItem(itm);
    }
    
    cbbo->subscribeEvent(Combobox::EventListSelectionAccepted, 
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
            {
                Combobox* combo = static_cast<Combobox*>(static_cast<const WindowEventArgs&>(args).window->getRootWindow()->getChild(combo_name));
                ListboxItem* item = combo->findItemWithText(combo->getText(), 0);
                if (item)
                {
                    zone_changed_signal_( item->getID()); 
                }

                return true;
    
            })
        ); 

    cbbo->setReadOnly(true);
    cbbo->setSortingEnabled(false);
    //cbbo->handleUpdatedListItemData();

}

vis_settings_panel_impl::~vis_settings_panel_impl()
{
}

void vis_settings_panel_impl::set_visible(bool visible)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    root->getChild(combo_name)->setVisible(visible);
    root->getChild(btn_exit_name)->setVisible(visible);
}

bool vis_settings_panel_impl::visible()
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    return root->getChild(btn_exit_name)->isVisible() || root->getChild(combo_name)->isVisible();
}


namespace app
{
    vis_settings_panel_ptr create_vis_settings_panel(const app::zones_t &zones )
    {
        return boost::make_shared<vis_settings_panel_impl>(  zones );
    }
}
