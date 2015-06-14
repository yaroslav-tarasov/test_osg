#include "stdafx.h"

#include <cegui/CEGUI.h>

#include "main_window_impl.h"

#include "application/main_window.h"
//#include "application/qt_utils.h"

#include "menu_impl.h"
//#include "toolbar_impl.h"
//#include "taskbar.h"
//#include "statusbar_impl.h"

using namespace CEGUI;

namespace
{

void show_panel_internal(widget_wrapper_impl *panel, boost::optional<bool> const &show)
{
    if (show)
    {
        if (*show)
            panel->show();
        else
            panel->hide();
    }
}

} // end of anonymous namespace


namespace app
{
	main_window_ptr create_main_win()
	{
		main_window_impl_ptr mw = boost::make_shared<main_window_impl>() ;
		return mw ;
	}
}

widget_wrapper_impl::widget_wrapper_impl(std::string const &name, app::widget_ptr w, CEGUI::Window *parent)
    : window_ (parent)
   // , main_layout_(new QHBoxLayout)
    , widget_     (w)
{
    //main_layout_->addWidget(app::qwidget(w.get())) ;
    //setLayout(main_layout_) ;
    //main_layout_->setContentsMargins(1, 1, 1, 1) ;
    //main_layout_->setSizeConstraint(QLayout::SetMinAndMaxSize) ;

    //setWindowFlags(Qt::Tool);
    //setWindowTitle(QString::fromUtf8(name.c_str())) ;
    //setObjectName(QString::fromUtf8(name.c_str())) ;

    
}

widget_wrapper_impl::~widget_wrapper_impl()
{
    //app::qwidget(widget_.get())->setParent(nullptr) ;
}

void widget_wrapper_impl::show() {};

void widget_wrapper_impl::hide() {};

//void widget_wrapper_impl::closeEvent(QCloseEvent *event)
//{
//    widget_->hide_signal_();
//    QWidget::closeEvent(event);
//}

main_window_impl::main_window_impl()
    : full_screen_(false)
    , taskbar_    (nullptr)
{
    //setWindowTitle(QCoreApplication::applicationName()) ;
    //setObjectName(QCoreApplication::organizationName() + "::" + QCoreApplication::applicationName() + "::" + "main_window_impl") ;
    //setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks) ;
    //setMouseTracking(true) ;
    GUIContext& context = System::getSingleton().getDefaultGUIContext();

    SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    // context.getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");

    FontManager::getSingleton().createFromFile( "DejaVuSans-10.font" );
    context.setDefaultFont( "DejaVuSans-10" );
    context.getDefaultFont()->setAutoScaled(CEGUI::ASM_Disabled);

    Window* root = WindowManager::getSingleton().createWindow( "DefaultWindow", "Root" );
    context.setRootWindow(root);

	menuBar = dynamic_cast<CEGUI::Menubar*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Menubar", "mainMenu"));
    
	root->addChild(menuBar);
	
	load_layout(boost::none) ;


}

main_window_impl::~main_window_impl()
{
    // Set parent to 0 to avoid call delete from QMainWindow destructor
    //foreach(app::widget_ptr p, widgets_)
    //    dynamic_cast<QWidget *>(p.get())->setParent(0) ;
}

void main_window_impl::add_doc_panel(std::string const &name, app::widget_ptr p, bool first_time_floating)
{
    add_panel_internal(name, p, first_time_floating) ;
}

void main_window_impl::add_panel(std::string const &name, app::widget_ptr p)
{
    add_panel_internal(name, p, boost::none) ;
}

void main_window_impl::add_central_panel(std::string const &name, app::widget_ptr p)
{
    //QString qname = QString::fromUtf8(name.c_str()) ;

    //if (QWidget *qp = dynamic_cast<QWidget *>(p.get()))
    //{
    //    setCentralWidget(qp) ;
    //    qp->show();
    //    qp->setObjectName(qname);

    //    widgets_.insert(p) ;
    //}
    //else
    //    LogTrace("main_window_impl::add_central_panel << " << "panel " << name  << "is not QWidget") ;
}

void main_window_impl::show_panel(app::widget_ptr p, bool show)
{
 /*   if (QWidget *qw = dynamic_cast<QWidget*>(p.get()))
        if (QWidget *ww = dynamic_cast<QWidget *>(qw->parent()))
        {
            if (show)
                ww->show() ;
            else
                ww->hide() ;

            auto it = binded_widgets_.find(p) ;
            if (binded_widgets_.end() != it)
                it->second.tb->set_checked(it->second.btn_id, show) ;
        }*/
}

void main_window_impl::set_visible	  (bool visible)
{
	//menuBar->setVisible(visible);
	GUIContext& context = System::getSingleton().getDefaultGUIContext();
	CEGUI::Window* root = context.getRootWindow();
	root->setVisible(visible);
}

bool  main_window_impl::visible ()
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
	CEGUI::Window* root = context.getRootWindow();
	return /*menuBar->*/root->isVisible();
}

//app::tool_bar_ptr main_window_impl::create_tool_bar(std::string const &name, unsigned qt_toolbar_area, bool standard)
//{
//    //QString qname = QString::fromStdString(name) ;
//
//    auto tb = boost::make_shared<tool_bar_impl>(this, standard, name) ;
//    //widgets_.insert(tb) ;
//
//    //tb->setObjectName(qname);
//    //addToolBar((Qt::ToolBarArea)qt_toolbar_area, tb.get()) ;
//
//    return tb ;
//}

//app::tool_bar_ptr main_window_impl::create_task_bar(const std::string &name, bool standard)
//{
//    //VerifyMsg(!taskbar_, "only one taskbar can be created in main_window_impl");
//
//    //auto tb = boost::make_shared<taskbar>(this, standard, name) ;
//    //widgets_.insert(tb) ;
//
//    //tb->setObjectName(QString::fromUtf8(name.c_str()));
//
//    //taskbar_ = tb.get();
//    //connect(taskbar_, SIGNAL(size_changed()), this, SLOT(move_taskbar()));
//    //move_taskbar();
//
//    return tb ;
//}

//app::status_bar_ptr main_window_impl::create_status_bar(std::string const &name)
//{
//    //QString qname = QString::fromUtf8(name.c_str()) ;
//
//    //status_bar_impl_ptr sb = boost::make_shared<status_bar_impl>(this) ;
//    //widgets_.insert(sb) ;
//
//    //sb->setObjectName(qname);
//    //setStatusBar(sb.get());
//
//    return sb ;
//}

app::menu_ptr main_window_impl::add_main_menu(std::string const &name)
{
    CEGUI::String skin = menuBar->getType();
	skin = skin.substr(0, skin.find_first_of('/'));
	CEGUI::String menuItemMapping = skin + "/MenuItem";
	CEGUI::String popupMenuMapping = skin + "/PopupMenu";

	CEGUI::WindowManager& windowManager = CEGUI::WindowManager::getSingleton(); 
	CEGUI::MenuItem* mi = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, name + "_MenuItem"));
	mi->setText(name);
	menuBar->addChild(mi);

	menu_impl_ptr m = boost::make_shared<menu_impl>(mi,popupMenuMapping,name) ;
	mi->addChild(m.get()->get_menu());

    widgets_.insert(m) ;

    // menuBar()->addMenu(m.get()) ;
    // m->setTitle(QString::fromUtf8(name.c_str())) ;

    return m ;
}

void main_window_impl::track_context_menu(std::string const &/*name*/, boost::function<void (app::menu &m)> fill_menu)
{
    //menu_impl m(this) ;

    //fill_menu(m) ;
    //QPoint p = QCursor::pos() ;
    //m.track(app::cg_point(p)) ;
}

void main_window_impl::remove_widget(app::widget_ptr w)
{
    //if (binded_widgets_.end() != binded_widgets_.find(w))
    //    unbind_panel(w) ;

    //if (QWidget *qw = dynamic_cast<QWidget*>(w.get()))
    //    if (QWidget *ww = dynamic_cast<QWidget *>(qw->parent()))
    //    {
    //        save_panel_state(ww) ;
    //        qw->setParent(NULL) ;
    //        delete ww ;
    //    }

    //if (dynamic_cast<QWidget*>(w.get()) == taskbar_)
    //    taskbar_ = nullptr;

    widgets_.erase(w) ;
}

app::widget* main_window_impl::find_widget(std::string const &name) const
{
 /*   QList<QWidget*> widgets = findChildren<QWidget*>(QString::fromUtf8(name.c_str())) ;
    foreach(QWidget* w, widgets)
    {
        if (QWidget *ww = dynamic_cast<QWidget *>(w))
        {
            QList<QWidget*> child_widgets = ww->findChildren<QWidget*>() ;
            foreach(QWidget* cw, child_widgets)
                if (app::widget *aw = dynamic_cast<app::widget *>(cw))
                    return aw ;
        }
        else if (app::widget *aw = dynamic_cast<app::widget *>(w))
            return aw ;
    }*/

    return NULL ;
}

void main_window_impl::load_layout(boost::optional<std::string> const &lname)
{
    //std::string layout_name = !lname ? default_layout_name() : *lname ;

    //QSettings settings ;
    //settings.beginGroup(layout_name.c_str());
    //    restoreGeometry(settings.value("window_geometry").toByteArray());
    //    restoreState(settings.value("window_state").toByteArray());
    //settings.endGroup();

    //for (auto it = widgets_.begin(), end = widgets_.end(); it != end; ++it)
    //    if (QWidget *ww = dynamic_cast<QWidget *>(app::qwidget(it->get())->parent()))
    //        load_panel_state(ww) ;
}

void main_window_impl::save_layout(boost::optional<std::string> const &lname) const
{
    //std::string layout_name = !lname ? default_layout_name() : *lname ;

    //QSettings settings ;
    //settings.beginGroup(layout_name.c_str());
    //    settings.setValue("window_geometry", saveGeometry());
    //    settings.setValue("window_state", saveState());
    //settings.endGroup();

    //for (auto it = widgets_.begin(), end = widgets_.end(); it != end; ++it)
    //    if (QWidget *ww = dynamic_cast<QWidget *>(app::qwidget(it->get())->parent()))
    //        save_panel_state(ww) ;
}

std::string main_window_impl::default_layout_name() const
{
    return "";//!full_screen_ ? objectName().toStdString() : (objectName().toStdString() + "_FULLSCREEN") ;
}

void main_window_impl::set_title(std::string const &title)
{
    //setWindowTitle(QCoreApplication::applicationName() + " - " + QString::fromUtf8(title.c_str())) ;
}

void main_window_impl::bind_panel(std::string const &toolbar_name, app::widget_ptr p)
{
    unbind_panel(p) ;

    //app::tool_bar_ptr tb ;

    //for (auto it = widgets_.begin(), end = widgets_.end(); it != end; ++it)
    //    if (app::qwidget(it->get())->objectName() == toolbar_name.c_str())
    //    {
    //        tb = boost::dynamic_pointer_cast<app::tool_bar>(*it) ;
    //        break ;
    //    }

    //if (!tb)
    //    return ;

    //if (widgets_.end() == widgets_.find(p))
    //{
    //    LogTrace("bind_panel_to_toolbar supports only added widgets") ;
    //    return ;
    //}

    //QWidget *qp = dynamic_cast<QWidget*>(p.get()) ;
    //QObject *pp = qp->parent() ;

    //if (widget_wrapper_impl *ww = dynamic_cast<widget_wrapper_impl *>(pp))
    //{
    //    std::string name = ww->objectName().toStdString() ;

    //    size_t show_btn_id = tb->add_button_named(boost::bind(&show_panel_internal, ww, _1), name, true) ;
    //    tb->set_checked(show_btn_id, ww->isVisible()) ;
    //    binded_widgets_.insert(make_pair(p, tb_btn(tb, show_btn_id))) ;

    //    p->subscribe_hide(boost::bind(&app::tool_bar::set_checked, tb, show_btn_id, false));
    //}
    //else
    //{
    //    LogTrace("bind_panel_to_toolbar doesn't support dockable widgets") ;
    //}
}

void main_window_impl::unbind_panel(app::widget_ptr p)
{
    auto it = binded_widgets_.find(p) ;
    if (binded_widgets_.end() != it)
    {
        it->second.tb->remove(it->second.btn_id) ;
        binded_widgets_.erase(p) ;
    }
}

void main_window_impl::set_quit_checker(boost::function<bool ()> qchecker)
{
    quit_checker_ = qchecker ;
}

void main_window_impl::set_full_screen(bool full_screen)
{
    if (full_screen_ != full_screen)
    {
        full_screen_ = full_screen;

        if (full_screen)
        {
            //setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
            //showFullScreen();
        }
        else
        {
            //setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            //showNormal();
        }
    }
}

void main_window_impl::add_panel_internal(std::string const &name, app::widget_ptr p, boost::optional<bool> first_time_floating)
{
    //QString qname = QString::fromUtf8(name.c_str()) ;

    //if (QWidget *qp = dynamic_cast<QWidget *>(p.get()))
    //{
    //    QWidget *wrapper_widget = nullptr ;

    //    if (first_time_floating)
    //    {
    //        QDockWidget* doc = new QDockWidget(qname, this) ;
    //        doc->setObjectName(qname) ;

    //        doc->setWidget(qp) ;

    //        if (!restoreDockWidget(doc))
    //        {
    //            addDockWidget(Qt::RightDockWidgetArea, doc, Qt::Vertical) ;
    //            doc->setFloating(*first_time_floating) ;
    //        }

    //        wrapper_widget = doc ;
    //    }
    //    else
    //    {
    //        wrapper_widget = new widget_wrapper_impl(name, p, this) ;
    //        wrapper_widget->setObjectName(qname) ;
    //    }

    //    Assert(wrapper_widget) ;
    //    load_panel_state(wrapper_widget) ;
    //    widgets_.insert(p) ;
    //}
    //else
    //    LogTrace("main_window_impl::add_panel << " << "panel " << name  << "is not QWidget") ;
}

void main_window_impl::load_panel_state(CEGUI::Window *w)
{
    //QSettings settings ;
    //settings.beginGroup(objectName());
    //settings.beginGroup(w->objectName());

    //QVariant vis = settings.value("visible") ;
    //if (!vis.isNull())
    //{
    //    if (vis.toBool())
    //        w->show() ;
    //    else
    //        w->hide() ;
    //}

    //QVariant pos = settings.value("pos") ;
    //if (!pos.isNull())
    //{
    //    QPoint p = pos.toPoint() ;
    //    w->move(p.x(), p.y()) ;
    //}

    //settings.endGroup();
    //settings.endGroup();
}

void main_window_impl::save_panel_state(CEGUI::Window *w) const
{
    //QSettings settings ;
    //settings.beginGroup(objectName());
    //settings.beginGroup(w->objectName());

    //settings.setValue("visible", w->isVisible());
    //settings.setValue("pos", w->pos());

    //settings.endGroup();
    //settings.endGroup();
}

//void main_window_impl::closeEvent(QCloseEvent * event)
//{
//    if (!quit_checker_ || quit_checker_())
//    {
//        on_close_event();
//        event->accept();
//    }
//    else
//        event->ignore();
//}

//void main_window_impl::changeEvent(QEvent *event)
//{
//    if (full_screen_ && event->type() == QEvent::WindowStateChange)
//    {
//        if (isMinimized())
//        {
//            showFullScreen() ;
//            return event->ignore() ;
//        }
//    }
//
//    return QMainWindow::changeEvent(event);
//}

//void main_window_impl::mouseMoveEvent(QMouseEvent *event)
//{
//    if (taskbar_ && event->buttons() == Qt::NoButton)
//    {
//        const auto &rect = geometry();
//        const auto &bottom_rect = rect.adjusted(0, rect.height() - taskbar::height, 0, 0);
//        const auto pos = mapToGlobal(event->pos());
//
//        if (bottom_rect.contains(pos))
//        {
//            if (!taskbar_->isVisible())
//                taskbar_->show();
//        }
//        else
//        {
//            if (taskbar_->isVisible())
//                taskbar_->hide();
//        }
//    }
//
//    QMainWindow::mouseMoveEvent(event);
//}
//
//void main_window_impl::resizeEvent(QResizeEvent *event)
//{
//    move_taskbar();
//    QMainWindow::resizeEvent(event);
//}
//
//void main_window_impl::moveEvent(QMoveEvent *event)
//{
//    move_taskbar();
//    QMainWindow::moveEvent(event);
//}

//void main_window_impl::move_taskbar()
//{
//    //if (taskbar_)
//    //{
//    //    const auto &rect = geometry();
//    //    const auto p = QPoint(rect.x() + (rect.width()-taskbar_->width())/2, rect.y() + rect.height() - taskbar::height);
//    //    taskbar_->move(p);
//    //}
//}

void main_window_impl::on_close_event()
{
    save_layout(boost::none) ;
    about_to_quit_signal_() ;
}
