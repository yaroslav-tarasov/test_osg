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
    size_t        add_string(std::wstring const &text, target const &click, target const &hover) override ;
    size_t        get_string(std::wstring const &text) const override ;

    app::menu_ptr add_pop_up   (std::wstring const &text) override ;
    void          add_separator() override ;
    void          set_enabled  (bool enabled) override ;
    void          set_checked  (size_t string_id, bool checked) override ;
    void          set_enabled  (size_t string_id, bool enabled) override ;
    void          set_shortcut (size_t string_id, unsigned qt_key) override ;
    void          remove       (size_t string_id) override ;

public:
    CEGUI::MenuBase * get_menu() { return base_; };
	const CEGUI::String&     get_name() { return dynamic_cast<CEGUI::Window*>(base_)->getName(); };

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
    std::unordered_map<CEGUI::MenuItem*, target>  actions_;
    std::unordered_map<CEGUI::MenuItem*, target>  hovers_;
	CEGUI::MenuBase *        base_   ;
    //QList
	std::list<menu_impl_ptr> popups_ ;
};
