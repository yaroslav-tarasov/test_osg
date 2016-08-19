#pragma once

#include "application/widget.h"
#include "application_fwd.h"

#include "common/event.h"

namespace app
{

struct menu : widget
{
    typedef boost::function<void ()> target;

    virtual size_t   add_string(std::wstring const &text, target const &click = target(), target const &hover = target()) = 0 ;
    virtual size_t   get_string(std::wstring const &text) const = 0 ;
    virtual menu_ptr add_pop_up(std::wstring const &text) = 0 ;
    virtual void     add_separator() = 0 ;

    virtual void     set_enabled(bool enabled) = 0 ;

    virtual void     set_checked(size_t string_id, bool checked) = 0 ;
    virtual void     set_enabled(size_t string_id, bool enabled) = 0 ;
    virtual void     set_shortcut(size_t string_id, unsigned qt_key) = 0 ;

    virtual void     remove(size_t string_id) = 0 ;

    DECLARE_EVENT(cursor_out, ()) ;
};

struct menu_holder
{
    virtual ~menu_holder(){}

    virtual void get_menu(menu &m, widget *called_from) = 0 ;
};

} // end of namespace app
