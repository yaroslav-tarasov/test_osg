#pragma once

//#include "chart/chart_engine_impl.h"

#include "application/main_window.h"

class widget_wrapper_impl
    /*: public CEGUI::Window*/
{

public:
    widget_wrapper_impl(std::string const &name, app::widget_ptr w, CEGUI::Window *parent = nullptr) ;
    ~widget_wrapper_impl() ;
	
	void show() ;
	void hide() ;
//private:
//    void closeEvent(QCloseEvent *event);

private:
    // QPointer<QHBoxLayout> main_layout_ ;
    app::widget_ptr       widget_ ;
	CEGUI::Window*        window_ ;  
};

class main_window_impl
    : public app::main_window
{

public:
    main_window_impl() ;
    ~main_window_impl() ;

private:
    // DECL_LOGGER("main_window_impl");

    // app::main_window
private:
    void add_doc_panel    (std::string const &name, app::widget_ptr p, bool first_time_floating) override ;
    void add_panel	      (std::string const &name, app::widget_ptr p) override ;
    void add_central_panel(std::string const &name, app::widget_ptr p) override ;
    void show_panel       (app::widget_ptr p, bool show) override ;

    //app::tool_bar_ptr   create_tool_bar  (std::string const &name, unsigned qt_toolbar_area, bool standard) override ;
    //app::tool_bar_ptr   create_task_bar  (std::string const &name, bool standard = true) override ;
    //app::status_bar_ptr create_status_bar(std::string const &name) override ;

    app::menu_ptr add_main_menu      (std::string const &name) override ;
    void          track_context_menu (std::string const &name, boost::function<void (app::menu &m)> fill_menu) override ;

    void          remove_widget (app::widget_ptr w) override ;
    app::widget*  find_widget   (std::string const &name) const override ;

    void load_layout      (boost::optional<std::string> const &lname) override ;
    void save_layout      (boost::optional<std::string> const &lname) const override ;

    void set_title        (std::string const &title) override ;

    void bind_panel       (std::string const &toolbar_name, app::widget_ptr p) override ;
    void unbind_panel     (app::widget_ptr p) override ;

    void set_quit_checker (boost::function<bool ()> qchecker) override ;
    void set_full_screen  (bool full_screen) override ;
	void set_visible	  (bool visible) override ;
	bool visible          ()             override ;

private:
    void add_panel_internal(std::string const &name, app::widget_ptr p, boost::optional<bool> first_time_floating) ;

    void load_panel_state (CEGUI::Window *w) ;
    void save_panel_state (CEGUI::Window *w) const ;

    std::string default_layout_name() const ;

    // QMainWindow
//private:
//    void closeEvent    (QCloseEvent  *event) override;
//    void changeEvent   (QEvent       *event) override;
//    void mouseMoveEvent(QMouseEvent  *event) override;
//    void resizeEvent   (QResizeEvent *event) override;
//    void moveEvent     (QMoveEvent   *event) override;

//private Q_SLOTS:
//    void move_taskbar();

public:
    void on_close_event() ;

private:
    typedef std::set<app::widget_ptr> widgets_set ;
    widgets_set widgets_ ;

private:
    struct tb_btn
    {
        tb_btn(app::tool_bar_ptr tb, size_t btn_id)
            : tb (tb)
            , btn_id(btn_id)
        {}

        app::tool_bar_ptr tb ;
        size_t btn_id ;
    };

    std::map<app::widget_ptr, tb_btn> binded_widgets_ ;

private:
    boost::function<bool ()>  quit_checker_ ;
    bool                      full_screen_;
    CEGUI::Window             *taskbar_;
	CEGUI::Menubar*			  menuBar  ;
};

typedef boost::shared_ptr<main_window_impl> main_window_impl_ptr ;
