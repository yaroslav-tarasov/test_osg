#include "stdafx.h"

#include <cegui/CEGUI.h>

#include "menu_impl.h"
//#include "application_impl.h"

//#include "application/document.h"

//#include "common/qt_dispatch.h"

using namespace CEGUI;

menu_impl::menu_impl(CEGUI::Window *parent,const String& type, const String& name)
{
    // connect(this, SIGNAL(hovered(QAction *)), this, SLOT(hovered_slot(QAction *))) ;
	base_ = dynamic_cast<CEGUI::MenuBase*>(CEGUI::WindowManager::getSingleton().createWindow(type, name));
}

size_t menu_impl::add_string(std::string const &text, target const &click, target const &hover)
{
    //QAction *act = addAction(QString::fromUtf8(text.c_str())) ;
    //connect(act, SIGNAL(triggered()), this, SLOT(action_slot())) ;

	CEGUI::String skin = base_->getType();
	skin = skin.substr(0, skin.find_first_of('/'));
	CEGUI::String menuItemMapping = skin + "/MenuItem";
	CEGUI::String popupMenuMapping = skin + "/PopupMenu";

	CEGUI::WindowManager& windowManager = CEGUI::WindowManager::getSingleton();
	auto menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, text + "_MenuItem"));
	menuItem->setText(text);
	base_->addItem(menuItem);

    if (click)
    //    actions_.insert(act, click) ;
	{
		menuItem->subscribeEvent(MenuItem::EventClicked, 
			Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
			{
				click(); 
				return true;
			})			
			
			); 	
	}	

    if (hover)
    //    hovers_.insert(act, hover) ;
	{
		menuItem->subscribeEvent(MenuItem::EventMouseEntersArea, 
			Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
			{
				hover(); 
				return true;
			})			
			
			); 	
	}

	size_t act;

    return (size_t)act ;
}

app::menu_ptr menu_impl::add_pop_up(std::string const &text)
{
    //popups_.append(boost::make_shared<menu_impl>((QWidget*)0)) ;
    //popups_.back()->setTitle(QString::fromUtf8(text.c_str())) ;

    base_->addChild(popups_.back().get()->get_menu()) ;
    return popups_.back() ;
}

void menu_impl::add_separator()
{
    //addSeparator() ;
}

void menu_impl::set_enabled(bool enabled)
{
    //auto keys_list = actions_.uniqueKeys() ;
    //for (auto it = keys_list.begin(), end = keys_list.end(); it != end; ++it)
    //    (*it)->setEnabled(enabled) ;
}

void menu_impl::set_checked(size_t string_id, bool checked)
{
    //QAction* act = (QAction*)string_id ;
    //if (actions_.contains(act))
    //{
    //    act->setCheckable(true) ;
    //    act->setChecked(checked) ;
    //}
}

void menu_impl::set_enabled(size_t string_id, bool enabled)
{
    //QAction* act = (QAction*)string_id ;
    //if (actions_.contains(act))
    //    act->setEnabled(enabled) ;
}

void menu_impl::set_shortcut(size_t string_id, unsigned qt_key)
{
    //QAction* act = (QAction*)string_id ;
    //if (actions_.contains(act))
    //    act->setShortcut(qt_key) ;
}

void menu_impl::remove(size_t string_id)
{
    //QAction* act = (QAction*)string_id ;
    //if (actions_.contains(act))
    //{
    //    removeAction(act) ;
    //    actions_.remove(act) ;
    //}
}

void menu_impl::track(cg::point_2i const &at)
{
    //Assert(nullptr != app_instance().doc()) ;
    //if (nullptr == app_instance().doc())
    //    return ;

    //if (actions_.empty() && popups_.empty())
    //    return ;

    //// Following curious code is used to correctly exit inner and main events loop
    //// when session is about to end
    //boost::function<void ()> def_call ;
    //app_instance().doc()->reset_inner_loop(
    //    [&def_call, this](boost::function<void ()> const &deferred_call)
    //{
    //    this->hide() ;
    //    def_call = deferred_call ;
    //}) ;
    //// end of curious code

    //exec(QPoint(at.x, at.y)) ;
    //app_instance().doc()->reset_inner_loop() ;

    //if (def_call)
    //    qt_dispatch::post(def_call);
}

//void menu_impl::leaveEvent(QEvent *event)
//{
//    QMenu::leaveEvent(event) ;
//
//    cursor_out_signal_() ;
//}

//void menu_impl::action_slot()
//{
//    target callable = actions_.value(qobject_cast<QAction*>(sender())) ;
//
//    if (callable)
//        callable() ;
//}

//void menu_impl::hovered_slot(QAction *act)
//{
//    target callable = hovers_.value(act) ;
//
//    if (callable)
//        callable() ;
//}
