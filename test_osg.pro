TEMPLATE = app

DEV_ROOT = C:/simex


OSG_DIR  = C:/Work/OSG
BULLET_DIR = C:/Work/bullet-2.82-r2704

PROJECT_FILE_NAME = $$replace(_PRO_FILE_, $$_PRO_FILE_PWD_/, )
PROJECT_NAME      = $$replace(PROJECT_FILE_NAME, .pro, )
TARGETX = x32

win32:{
contains(QMAKE_TARGET.arch, x86_64):{
 message("Platform x64")
 CONFIG += target_x86_64
 TARGETX = x64
} else {
 message("Platform x86")
 CONFIG += target_x86
}
}

INCLUDE_PATH = $$DEV_ROOT/src/_Include


#paths
INCLUDE_PATH = $$DEV_ROOT/src/_Include
EXTS         = $$DEV_ROOT/ext
BINS         = $$DEV_ROOT/bin

target_x86_64:{
 EXT_LIB      = $$EXTS/lib64
 BINS = $$BINS"64"
} else {
 EXT_LIB  = $$EXTS/lib
}

message($$EXT_LIB)

EXT_INCLUDE  = $$EXTS/include

##########################
# vcproj generation
contains(TEMPLATE, lib)|contains(TEMPLATE, app) {
    CONFIG -= debug_and_release
    CONFIG -= debug_and_release_target
} else:contains(TEMPLATE, vclib)|contains(TEMPLATE, vcapp) {
    CONFIG += debug_and_release
}

##########################
# debug/release management
CONFIG(debug,debug|release){
    BUILD_PATH = $$BINS/debug
    CONFIG    += debug
    CONFIG    -= release

    DEBUG_CFG   = True
    PATH_SUFFIX = debug

    unix:QMAKE_CXXFLAGS += -O0
    win32:QMAKE_CXXFLAGS += -Od

    INCLUDEPATH += $$EXT_INCLUDE/vld
    LIBS += -L$$EXT_LIB/vld

    message("Debug Configuration")
} else {
    BUILD_PATH = $$BINS/release
    CONFIG    += release
    CONFIG    -= debug

    RELEASE_CFG = True
    PATH_SUFFIX = release


    unix:QMAKE_CXXFLAGS += -O2
    win32:QMAKE_CXXFLAGS += -Ob2 -Oi -Ot /Oy- -fp:fast -fp:except-

    DEFINES *= NDEBUG
    win32:QMAKE_LFLAGS *= /DEBUG # for generating PDB files

    message("Release Configuration")
}

MISC_PATH = $$BUILD_PATH/$$PROJECT_NAME

#############
# build paths
OUT_PWD     = $$MISC_PATH
DESTDIR     = $$BUILD_PATH
OBJECTS_DIR = $$MISC_PATH
MOC_DIR     = $$MISC_PATH
UI_DIR      = $$MISC_PATH
RCC_DIR     = $$MISC_PATH

#############
# include paths

INCLUDEPATH += ./
INCLUDEPATH += $$INCLUDE_PATH
INCLUDEPATH += $$EXT_INCLUDE
INCLUDEPATH += $$EXT_INCLUDE/gmp
INCLUDEPATH += $$_PRO_FILE_PWD_
INCLUDEPATH += \
               $$PWD/ext/pugixml-1.4/src \
               $$(OSG_DIR)/3rdparty/include \
               $$(OSG_DIR)/OpenSceneGraph-3.2.1/build/include \
               $$(OSG_DIR)/OpenSceneGraph-3.2.1/include \
               $$(BULLET_DIR)/src \
               $$(OSG_DIR)/SPARK-1.5.5/include \
               $$PWD/common \
               $$PWD/bada

#############
# lib paths

LIBS += -L$$EXT_LIB/boost
LIBS += -L$$EXT_LIB/qt/$$PATH_SUFFIX
LIBS += -L$$(OSG_DIR)/OpenSceneGraph-3.2.1/build/lib
LIBS += -L$$BINS/$$PATH_SUFFIX
win32:LIBS += -L$$(BULLET_DIR)/build/lib/$$PATH_SUFFIX
LIBS += -L$$PWD/ext/pugixml-1.4/scripts/vs2010/$$TARGETX
LIBS += -L$$(OSG_DIR)/SPARK-1.5.5/lib/vc2008/static
LIBS += -L$$(OSG_DIR)/3rdparty/lib

CONFIG(debug,debug|release){
LIBS += -lpugixmld
} else {
LIBS += -lpugixml
}

message($$LIBS)

win32:{
DEFINES -= UNICODE
DEFINES += DISABLE_ROBUST_COMPUTATIONS
DEFINES += BOOST_ALL_NO_LIB
}

######################
# common configuration
CONFIG *= precompile_header
CONFIG -= qt qt_no_framework

PRECOMPILED_HEADER = precompiled.h
DEFINES += precompile_header

LIBS += -Wl,--start-group -lasync_services -llogger -lalloc -Wl,--end-group

win32{
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    QMAKE_CXXFLAGS += -Zc:wchar_t

#    equals(RELEASE_CFG, True):QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_MT_DLL
#    equals(DEBUG_CFG  , True):QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_MT_DLLDBG

    #correct versions for Windows 7
    QMAKE_CXXFLAGS += -DWINVER=0x0601
    QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0601
    QMAKE_CXXFLAGS += -DWIN32_LEAN_AND_MEAN

    # non dll-interface class ...
    QMAKE_CXXFLAGS += /wd4275
    # ... class 'boost::shared_ptr<T>' needs to have dll-interface to be used by clients of struct ...
    QMAKE_CXXFLAGS += /wd4251
    # ... 'this' : used in base member initializer list
    QMAKE_CXXFLAGS += /wd4355

    QMAKE_CXXFLAGS += -DNOMINMAX

    QMAKE_CXXFLAGS += /MP8
    QMAKE_CXXFLAGS -= -Zm200
    QMAKE_CXXFLAGS *= -Zm400

    LIBS += /NODEFAULTLIB:libcmt.lib
}


CONFIG += qt
QT += core gui

HEADERS += \
    stdafx.h \
    phys/RigidUpdater.h \
    phys/rigid_body_info.h \
    phys/phys_sys_common.h \
    phys/GLDebugDrawer.h \
    phys/BulletInterface.h \
    phys/bullet_helpers.h \
    av/Terrain.h \
    find_node_visitor.h \
    creators.h \
    tests/client.h

SOURCES += \
    test_osg.cpp \
    vehicle_model_states.cpp \
    vehicle_model.cpp \
    #static_convex.cpp \
    shaders.cpp \
    nodes_manager.cpp \
    nodes_management.cpp \
    node_impl.cpp \
    materials.cpp \
    dubins.cpp \
    bada_import.cpp \
    animation_handler.cpp \
    aircraft_visual.cpp \
    aircraft_shassis_impl.cpp \
    phys/aircraft_phys.cpp \
    aircraft_model.cpp \
    phys/RigidUpdater.cpp \
    phys/GLDebugDrawer.cpp \
    phys/BulletInterface.cpp \
    phys/ray_cast_vehicle.cpp \
    phys/phys_aircraft.cpp \
    phys/bvh_static_mesh.cpp \
    nfi/lib_loader.cpp \
    sm/ViewDependentShadowMap.cpp \
    sm/ShadowTechnique.cpp \
    sm/ShadowSettings.cpp \
    sm/ShadowMap.cpp \
    sm/ShadowedScene.cpp \
    av/Terrain.cpp \
    av/Scene.cpp \
    av/PreRender.cpp \
    av/Object.cpp \
    av/LOD.cpp \
    av/FogLayer.cpp \
    av/Ephemeris.cpp \
    av/EnvRenderer.cpp \
    av/CloudLayer.cpp \
    find_node_visitor.cpp \
    creators.cpp \
    spark/SparkUpdatingHandler.cpp \
    spark/SparkDrawable.cpp \
    spark/simple_effect.cpp \
    spark/rain_effect.cpp \
    spark/fire_effect.cpp \
    spark/explosion_effect.cpp \
    spark/osgspark.cpp \
    main_scene2.cpp \
    tests/test_network.cpp \
    tests/sync_qt.cpp
