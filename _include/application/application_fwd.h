#pragma once

#include "common/dyn_lib.h"

namespace app
{

struct widget;
struct tool_bar;
struct status_bar;

struct menu;
struct menu_holder;

struct main_window;
struct application;

typedef boost::shared_ptr<widget>     widget_ptr ;

typedef boost::shared_ptr<tool_bar>   tool_bar_ptr ;
typedef boost::shared_ptr<status_bar> status_bar_ptr ;

typedef boost::shared_ptr<menu>        menu_ptr ;
typedef boost::shared_ptr<menu_holder> menu_holder_ptr ;

typedef boost::shared_ptr<main_window> main_window_ptr ;
typedef boost::shared_ptr<application> application_ptr ;

main_window_ptr create_main_win();

} // end of namespace app

#if !defined(STATIC_BASAPPLIC_API)
#if defined(GUI_ATTR_LIB) || defined(GUI_BASE_LIB) || defined(GUI_IPO_LIB) || defined(GUI_ATC_LIB) || defined(PARKING_LIB) || defined(ANI_LOADER_LIB) || defined(GUI_AV_LIB)
# define BASAPPLIC_API __HELPER_DL_EXPORT
#else
# define BASAPPLIC_API __HELPER_DL_IMPORT
#endif
#else 
# define BASAPPLIC_API 
#endif
