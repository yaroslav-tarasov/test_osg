#include "stdafx.h"

#pragma optimize( "", off )

// Ну и куда то убрать надо 
FIXME(Заголовок как в рефренсном)
#include <cegui/CEGUI.h>

#include "vis_settings_panel_impl.h"

using namespace CEGUI;

#include "cegui_utils.h"


namespace {

const  String combo_name     ( "selModeBox"		);
const  String btn_exit_name  ( "btnExit"		);
const  String main_menu_name ( "mainMenuBar"    );
const  String tb_name		 ( "tbLight"        );
const  String setting_dlg    ( "FrameWindow"    );


std::string trim_copy (const char* name )
{
    return boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtRadX"));
}

}

vis_settings_panel_impl::vis_settings_panel_impl(  const app::zones_t &zones, const app::settings_t& s )
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    
	WindowManager& winMgr = WindowManager::getSingleton();

    FrameWindow* mainWindow = static_cast<FrameWindow*>(
        CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "MainWindow") );
    mainWindow->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim(0.5f)));
    //demoWindow->setSize(USize(cegui_reldim(0.3f), cegui_reldim(0.3f)));
    mainWindow->setMinSize(USize(cegui_reldim(0.1f), cegui_reldim(0.1f)));
    mainWindow->setText( "Choose Zone" );

	auto fn_exit = [=](const CEGUI::EventArgs& args)->bool 
        {
			CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
			CEGUI::Window* root = context.getRootWindow();
			root->getChild("MainWindow")->setVisible(false);
            return true;

        };

    mainWindow->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked,Event::Subscriber(fn_exit));


#if 0
	PushButton* btnExit = static_cast<PushButton*>(
        WindowManager::getSingleton().createWindow("TaharezLook/Button", btn_exit_name) );

    root->addChild(btnExit);
    btnExit->setPosition(UVector2(cegui_reldim(0.95f), cegui_reldim(0.95f)));
    btnExit->setSize( USize(cegui_reldim(0.04f), cegui_reldim(0.02f)) );
    btnExit->setText( "Exit" );
    
    btnExit->subscribeEvent(PushButton::EventClicked, 
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
        {

            exit_app_signal_(); 
            return true;

        })
        ); 
#endif
    

    Combobox* cbbo = static_cast<Combobox*>( CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox", combo_name));
    cbbo->setPosition(UVector2(cegui_reldim(0.15f), cegui_reldim( 0.1f)));
    //cbbo->setSize(USize(cegui_reldim(0.66f), cegui_reldim( 0.33f)));
    mainWindow->addChild(cbbo);
    root->addChild( mainWindow );

    mainWindow->setVisible(false);

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
                Combobox* combo = static_cast<Combobox*>(static_cast<const WindowEventArgs&>(args).window->getRootWindow()->getChild("MainWindow/" + combo_name));
                
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

	CEGUI::ToggleButton*    checkbox = static_cast<ToggleButton*>( CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", tb_name));
	checkbox->setSelected(true);
	checkbox->setText( "Lights" );
	mainWindow->addChild(checkbox);
	
    auto settingsWindow = winMgr.loadLayoutFromFile("vis_settings.layout");
	root->addChild(settingsWindow);
	subscribeEvent(setting_dlg + "/Settings/chkLights", ToggleButton::EventSelectStateChanged,
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
        {
            
			bool wrap = isCheckboxSelected(setting_dlg + "/Settings/chkLights");
			set_lights_signal_(wrap); 
            return true;

        }));
    
    settingsWindow->setVisible(false);

	subscribeEvent(setting_dlg + "/LWeather/edtGrassMap", CEGUI::Editbox::EventTextAccepted,
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
	{

		std::string param = getEditboxText(setting_dlg + "/LWeather/edtGrassMap");
		set_map_signal_(boost::lexical_cast<float>(param.empty()?"0":param)); 
		return true;

	}));

	subscribeEvent(setting_dlg + "/LWeather/btnApply", PushButton::EventClicked,
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
	{

		std::string param = getEditboxText(setting_dlg + "/LWeather/edtGrassMap");
		set_map_signal_(boost::lexical_cast<float>(param.empty()?"0":param)); 
		return true;

	}));

    subscribeEvent(setting_dlg + "/Settings/chkShadows", ToggleButton::EventSelectStateChanged,
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
    {

        bool wrap = isCheckboxSelected(setting_dlg + "/Settings/chkShadows");
        set_shadows_signal_(wrap); 
        return true;

    }));
    
    subscribeEvent(setting_dlg + "/Settings/chkShadowsParticles", ToggleButton::EventSelectStateChanged,
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
    {

        bool wrap = isCheckboxSelected(setting_dlg + "/Settings/chkShadowsParticles");
        set_shadows_part_signal_(wrap); 
        return true;

    }));

    setItemText(setting_dlg + "/LWeather/edtRadX"    , boost::str(boost::format("%.2f") % s.clouds[0].radius_x));
    setItemText(setting_dlg + "/LWeather/edtRadY"    , boost::str(boost::format("%.2f") % s.clouds[0].radius_y));
    setItemText(setting_dlg + "/LWeather/edtX"       , boost::str(boost::format("%.2f") % s.clouds[0].x));
    setItemText(setting_dlg + "/LWeather/edtY"       , boost::str(boost::format("%.2f") % s.clouds[0].y));
    setItemText(setting_dlg + "/LWeather/edtHeight"  , boost::str(boost::format("%.2f") % s.clouds[0].height));
    setItemText(setting_dlg + "/LWeather/edtIntensity", boost::str(boost::format("%.2f") % s.clouds[0].intensity));
    setItemText(setting_dlg + "/GWeather/edtIntensity", boost::str(boost::format("%.2f") % s.intensity));



    auto cloud_settings_callback =
    [=](const CEGUI::EventArgs& args)->bool 
    {
        app::cloud_params_t s;

        std::string edtRadX = boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtRadX"));
        s.radius_x = boost::lexical_cast<float>(edtRadX.empty()?"0":edtRadX);

        std::string edtRadY = boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtRadY"));
        s.radius_y = boost::lexical_cast<float>(edtRadY.empty()?"0":edtRadY);

        std::string edtX = boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtX"));
        s.x = boost::lexical_cast<float>(edtX.empty()?"0":edtX);

        std::string edtY = boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtY"));
        s.y = boost::lexical_cast<float>(edtY.empty()?"0":edtY);

        std::string edtHeight = boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtHeight"));
        s.height = boost::lexical_cast<float>(edtHeight.empty()?"0":edtHeight);

        std::string edtIntensity = boost::trim_copy(getEditboxText(setting_dlg + "/LWeather/edtIntensity"));
        s.intensity = boost::lexical_cast<float>(edtIntensity.empty()?"0":edtIntensity);

        s.p_type   = 1;

        set_cloud_param_signal_(s); 
        return true;

    };

    subscribeEvent(setting_dlg + "/LWeather/edtRadX", Editbox::EventTextAccepted,
        Event::Subscriber(cloud_settings_callback));

    subscribeEvent(setting_dlg + "/LWeather/edtRadY", Editbox::EventTextAccepted,
        Event::Subscriber(cloud_settings_callback)); 

    subscribeEvent(setting_dlg + "/LWeather/edtX"   , Editbox::EventTextAccepted,
        Event::Subscriber(cloud_settings_callback)); 

    subscribeEvent(setting_dlg + "/LWeather/edtY"   , Editbox::EventTextAccepted,
        Event::Subscriber(cloud_settings_callback)); 
    
    subscribeEvent(setting_dlg + "/LWeather/edtHeight", Editbox::EventTextAccepted,
        Event::Subscriber(cloud_settings_callback)); 

    subscribeEvent(setting_dlg + "/LWeather/edtIntensity", Editbox::EventTextAccepted,
        Event::Subscriber(cloud_settings_callback)); 

    subscribeEvent(setting_dlg + "/GWeather/edtIntensity", Editbox::EventTextAccepted,
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
    {
        std::string param = boost::trim_copy(getEditboxText(setting_dlg + "/GWeather/edtIntensity"));
        set_global_intensity_signal_ (boost::lexical_cast<float>(param.empty()?"0":param)); 
        return true;

    }));

}

vis_settings_panel_impl::~vis_settings_panel_impl()
{
}

void vis_settings_panel_impl::set_visible(bool visible)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();

	if(root->getChild(btn_exit_name)) root->getChild(btn_exit_name)->setVisible(visible);
    if(root->getChild(setting_dlg))root->getChild(setting_dlg)->setVisible(visible);
}

bool vis_settings_panel_impl::visible()
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    return (root->getChild(btn_exit_name)?root->getChild(btn_exit_name)->isVisible():false) ||    root->getChild(setting_dlg)?root->getChild(setting_dlg)->isVisible():false; 
}

void vis_settings_panel_impl::set_light(bool on)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    if (root->isChild(setting_dlg + "/Settings/chkLights"))
    {
        ToggleButton* button = static_cast<ToggleButton*>(root->getChild(setting_dlg + "/Settings/chkLights"));
        return button->setSelected(on);
    }
}

void vis_settings_panel_impl::init_menu_bar(CEGUI::Menubar* menuBar)
{
	CEGUI::String skin = menuBar->getType();
	skin = skin.substr(0, skin.find_first_of('/'));
	CEGUI::String menuItemMapping = skin + "/MenuItem";
	CEGUI::String popupMenuMapping = skin + "/PopupMenu";

	CEGUI::WindowManager& windowManager = CEGUI::WindowManager::getSingleton(); 
	CEGUI::MenuItem* fileMenuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileMenuItem"));
	fileMenuItem->setText("File");
	menuBar->addChild(fileMenuItem);

	CEGUI::PopupMenu* filePopupMenu = static_cast<CEGUI::PopupMenu*>(windowManager.createWindow(popupMenuMapping, "FilePopupMenu"));
	fileMenuItem->addChild(filePopupMenu);

	CEGUI::MenuItem* menuItem;
	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileTestMenuItem1"));
	menuItem->setText("Open");
	filePopupMenu->addItem(menuItem);

	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileTestMenuItem2"));
	menuItem->setText("Save");
	filePopupMenu->addItem(menuItem);

	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileTestMenuItem3"));
	menuItem->setText("Exit");
	filePopupMenu->addItem(menuItem);

	menuItem->subscribeEvent(MenuItem::EventClicked, 
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
		{

			exit_app_signal_(); 
			return true;

		})
		); 

	CEGUI::MenuItem* viewMenuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "ViewMenuItem"));
	viewMenuItem->setText("View");
	menuBar->addChild(viewMenuItem);

	CEGUI::PopupMenu* viewPopupMenu = static_cast<CEGUI::PopupMenu*>(windowManager.createWindow(popupMenuMapping, "ViewPopupMenu"));
	viewMenuItem->addChild(viewPopupMenu);

	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "ViewTestMenuItem1"));
	menuItem->setText("Lights");
	viewPopupMenu->addItem(menuItem);
}

namespace app
{
    vis_settings_panel_ptr create_vis_settings_panel(const app::zones_t &zones, const app::settings_t& s )
    {
        return boost::make_shared<vis_settings_panel_impl>(  zones, s );
    }
}
