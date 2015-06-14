#pragma once

#include "application/menu.h"

class menu_impl ;
typedef shared_ptr<menu_impl> menu_impl_ptr ;

class menu_impl 
    : public app::menu
{

public:
    typedef app::menu::target target ;

    menu_impl(CEGUI::Window *parent, const CEGUI::String& type, const CEGUI::String& name) ;

    // app::menu
public:
    size_t        add_string(std::string const &text, target const &click, target const &hover) override ;
    app::menu_ptr add_pop_up(std::string const &text) override ;
    void          add_separator() override ;
    void          set_enabled(bool enabled) override ;
    void          set_checked(size_t string_id, bool checked) override ;
    void          set_enabled(size_t string_id, bool enabled) override ;
    void          set_shortcut(size_t string_id, unsigned qt_key) override ;
    void          remove(size_t string_id) override ;

public:
	CEGUI::MenuBase * get_menu() { return base_; };

    // QWidget
//private:
//    void leaveEvent(QEvent *event) override ;

    // own
public:
    void track(cg::point_2i const &at) ;

//private Q_SLOTS:
//    void action_slot() ;
//    void hovered_slot(QAction *act) ;

private:
    //QHash<QAction*, target> actions_ ;
    //QHash<QAction*, target> hovers_ ;
	CEGUI::MenuBase *        base_   ;
    //QList
	std::list<menu_impl_ptr> popups_ ;
};
