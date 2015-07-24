TEMPLATE = lib
SOLUTION_ROOT = ./../..
DEV_ROOT = $$(SIMEX_DIR)

include($$SOLUTION_ROOT/base.pri)

INCLUDEPATH += $$SOLUTION_ROOT

CONFIG += qt
QT += core gui opengl

DEFINES += OSGWIDGET_EXPORTS

PRECOMPILED_HEADER = stdafx.h

HEADERS += \
    stdafx.h \
    OSGWidget.h

SOURCES += \
    OSGWidget.cpp
