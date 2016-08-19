#pragma once

#include "application_fwd.h"
#include "application/widget.h"

#include "common/event.h"

namespace app
{

struct tool_bar
    : widget
{
    typedef boost::function<void (boost::optional<bool> const &)> target;

    virtual size_t add_button        (target const &tgt, std::string const &resource_icon_name, bool checkable = false)             = 0 ;
    virtual size_t add_button        (target const &tgt, unsigned int qt_standard_icon, bool checkable = false)                     = 0 ; // qstyle.h -> enum StandardPixmap
    virtual size_t add_button_named  (target const &tgt, std::string const &name, bool checkable = false)                           = 0 ;
    virtual size_t add_widget        (target const &tgt, app::widget_ptr widget)                                                    = 0 ;
    virtual size_t add_separator     ()                                                                                             = 0 ;

    virtual void set_checked(size_t button_id, bool checked) = 0 ;
    virtual void set_enabled(size_t button_id, bool enabled) = 0 ;

    virtual void set_icon(size_t button_id, std::string const &resource_icon_name) = 0 ;
    virtual void set_action(size_t button_id, target const &tgt) = 0 ;

    virtual void remove     (size_t button_id)               = 0 ;
};

struct status_bar
    : widget
{
    virtual void set_text(std::string const &text) = 0 ;
};

struct main_window
    : widget
{
    virtual void add_doc_panel    (std::string const &name, widget_ptr p, bool first_time_floating = false) = 0 ;
    virtual void add_panel        (std::string const &name, widget_ptr p)                                   = 0 ;
    virtual void add_central_panel(std::string const &name, widget_ptr p)                                   = 0 ;
    virtual void show_panel       (widget_ptr p, bool show)                                                 = 0 ;
	virtual void set_visible	  (bool visible)															= 0 ;
	virtual bool visible          ()																		= 0 ;

    //virtual tool_bar_ptr   create_tool_bar  (std::string const &name, unsigned qt_toolbar_area, bool standard = true) = 0 ; // qnamespace.h -> enum ToolBarArea
    //virtual tool_bar_ptr   create_task_bar  (std::string const &name, bool standard = true) = 0 ;
    //virtual status_bar_ptr create_status_bar(std::string const &name) = 0 ;

    virtual menu_ptr add_main_menu (std::wstring const &name) = 0 ;
    virtual menu_ptr get_main_menu (std::wstring const &name) = 0 ;
    virtual void track_context_menu(std::wstring const &name, boost::function<void (menu &m)> fill_menu) = 0 ;

    virtual void    remove_widget(app::widget_ptr w) = 0 ;
    virtual widget* find_widget(std::string const &name) const = 0 ;

    virtual void load_layout(boost::optional<std::string> const &lname = /*boost::*/none) = 0 ;
    virtual void save_layout(boost::optional<std::string> const &lname = /*boost::*/none) const = 0 ;

    virtual void set_title(std::wstring const &title) = 0 ;

    virtual void bind_panel(std::string const &toolbar_name, widget_ptr p) = 0 ;
    virtual void unbind_panel(widget_ptr p) = 0 ;

    virtual void set_quit_checker(boost::function<bool ()> qchecker) = 0 ;
    virtual void set_full_screen(bool full_screen) = 0 ;

    DECLARE_EVENT(about_to_quit, ()) ;
};

} // end of namespace app
