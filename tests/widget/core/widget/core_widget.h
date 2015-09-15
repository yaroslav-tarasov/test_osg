#pragma once

#include <QObject>
//@#include <QtWidgets>
//@#include <QtGui>
#include <QWidget>

#include <QGuiApplication>

namespace core {
//
// Core widget base
//

class CoreWidget : public QWidget
{
    Q_OBJECT

public:

    CoreWidget();

    void reattach_to_window( WId window );

protected:

#ifdef WIN32
    QPaintEngine * paintEngine() const { return nullptr; }
#endif
};

}
