TEMPLATE = app
win32:TEMPLATE = vcapp

DEV_ROOT = $$(SIMEX_DIR)
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
               $$(SIMEX_DIR)/src/_Include                     \
               $$(SIMEX_DIR)/src/_Include/network             \
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


#############
# lib paths

LIBS += -L$$EXT_LIB/boost
LIBS += -L$$EXT_LIB/qt/$$PATH_SUFFIX
LIBS += -L$$(OSG_DIR)/OpenSceneGraph-3.2.1/build/lib
LIBS += -L$$BINS/$$PATH_SUFFIX
win32:LIBS += -L$$(BULLET_DIR)/build/lib/$$PATH_SUFFIX
#LIBS += -L$$PWD/ext/pugixml-1.4/scripts/vs2010/$$TARGETX
LIBS += -L$$(OSG_DIR)/SPARK-1.5.5/lib/vc2008/static
LIBS += -L$$(OSG_DIR)/3rdparty/lib
LIBS += -L$$(BULLET_DIR)/lib
LIBS += -L$$(SIMEX_DIR)/ext/lib/tinyxml2/$$PATH_SUFFIX
LIBS += -L$$(SIMEX_DIR)/bin/$$PATH_SUFFIX

CONFIG(debug,debug|release){
LIBS += -lpugixmld -ltinyxml2 -lnet_layer -lfms -llogger -lasync_services -lalloc -lmeteo
} else {
LIBS += -lpugixml -ltinyxml2 -lnet_layer -lfms -llogger -lasync_services -lalloc -lmeteo
}

win32:{
DEFINES -= UNICODE
DEFINES += DISABLE_ROBUST_COMPUTATIONS
DEFINES += BOOST_ALL_NO_LIB
DEFINES += CG_PRIMITIVES
}


CONFIG += qt
QT += core gui

SOURCES += \
		tests/qt/QtOSG.cpp \
		tests/qt/MainWindow.cpp 	

HEADERS += \
		tests/qt/QtOSG.h \
		tests/qt/MainWindow.h 			
			
LIBS += -losgqtwidget
			


