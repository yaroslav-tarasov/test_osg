TEMPLATE = app
win32:TEMPLATE = vcapp

SIMEX_DIR_FULL = $$(SIMEX_DIR)_qt5
DEV_ROOT = $$SIMEX_DIR_FULL
SOLUTION_ROOT = .

include($$SOLUTION_ROOT/base.pri)

include(test_osg.pri)

#############
# include paths

INCLUDEPATH += ./
INCLUDEPATH += \
               $$PWD/utils                                    \
               $$PWD/_include                                 \
               $$PWD/_include/objects                         \
               $$PWD/objects                                  \
               $$(SIMEX_DIR_FULL)/src/_Include                \
               $$(SIMEX_DIR_FULL)/src/_Include/network        \
               $$PWD/ext/pugixml-1.4/src                      \
               $$(OSG_DIR)/3rdparty/include                   \
               $$(OSG_DIR)/OpenSceneGraph-3.2.1/build/include \
               $$(OSG_DIR)/OpenSceneGraph-3.2.1/include       \
               $$(BULLET_DIR)/src                             \
               $$(OSG_DIR)/SPARK-1.5.5/include                \
               $$PWD/common                                   \
               $$PWD/bada
INCLUDEPATH += $$INCLUDE_PATH
INCLUDEPATH += $$EXT_INCLUDE
INCLUDEPATH += $$EXT_INCLUDE/gmp
INCLUDEPATH += $$_PRO_FILE_PWD_

win32:{
DEFINES -= UNICODE
DEFINES += DISABLE_ROBUST_COMPUTATIONS
DEFINES += BOOST_ALL_NO_LIB
DEFINES += CG_PRIMITIVES
}


CONFIG += qt
QT += core gui widgets

SOURCES += \
		tests/qt/QtOSG.cpp \
		tests/qt/MainWindow.cpp 	

SOURCES -= $$PWD/test_osg.cpp       
SOURCES += $$PWD/vis_app.cpp

HEADERS += \
		tests/qt/QtOSG.h \
		tests/qt/MainWindow.h 			
			
LIBS += -losgqtwidget
			


