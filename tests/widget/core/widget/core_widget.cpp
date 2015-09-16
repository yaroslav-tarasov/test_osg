#include "stdafx.h"

namespace core {

CoreWidget::CoreWidget()
    : QWidget(nullptr, Qt::MSWindowsOwnDC)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_LayoutOnEntireRect);
    setAutoFillBackground(false);
}

void CoreWidget::reattach_to_window( WId window )
{
    QWidget::create(window, true, true);
}

}