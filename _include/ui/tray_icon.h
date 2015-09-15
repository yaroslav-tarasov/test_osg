#pragma once

//#include "common/qt.h"

class tray_icon 
    : public QSystemTrayIcon
{
    Q_OBJECT

public:
    tray_icon(std::string const& icon_path, QObject *parent = 0)
        : QSystemTrayIcon(QIcon(icon_path.c_str()), parent)
        , menu_          (new QMenu())
    {
        auto exit_action_ = new QAction("Exit", menu_.get());    

        connect(exit_action_, SIGNAL(triggered()), this, SIGNAL(menu_exit()));

        menu_->addSeparator();
        menu_->addAction   (exit_action_);    

        setContextMenu(menu_.get());
    }

    void set_tooltip(std::string const& msg)
    {
        setToolTip(msg.c_str());
    }

Q_SIGNALS:
    void menu_exit();
    
private:
    unique_ptr<QMenu>  menu_;
};
