TEMPLATE = lib
SOLUTION_ROOT = ./../..
DEV_ROOT = $$(SIMEX_DIR)_qt5
SIMEX_DIR_FULL = $$(SIMEX_DIR)_qt5

include($$SOLUTION_ROOT/base.pri)

INCLUDEPATH += $$SOLUTION_ROOT
#############
# include paths

INCLUDEPATH += ./
INCLUDEPATH += \
               $$(SIMEX_DIR_FULL)/src/_Include                \
               $$(SIMEX_DIR_FULL)/src/_Include/network        \
               $$(BULLET_DIR)/src                             \
               $$(OSG_DIR)/SPARK-1.5.5/include                \
               $$SOLUTION_ROOT/common                         \
               $$SOLUTION_ROOT/bada
INCLUDEPATH += $$INCLUDE_PATH
INCLUDEPATH += $$EXT_INCLUDE
INCLUDEPATH += $$EXT_INCLUDE/gmp
INCLUDEPATH += $$_PRO_FILE_PWD_

INCLUDEPATH += \ 
               $$(OSG_DIR)/cegui-0.8.4/cegui/include \
               $$(OSG_DIR)/cegui-0.8.4/build/cegui/include 
               
LIBS += \               
 -L$$(OSG_DIR)/cegui-0.8.4/build/lib \

CONFIG += qt
QT += core gui opengl

DEFINES += OSGWIDGET_EXPORTS
# For spark only
DEFINES += QT_NO_EMIT

PRECOMPILED_HEADER = precompiled.h

HEADERS += \
    stdafx.h \
    OSGWidget.h

SOURCES += \
    OSGWidget.cpp

SOURCES += \   
#av    \
C:/Work/prj/test_osg/av/CloudLayer.cpp    \
C:/Work/prj/test_osg/av/Database.cpp    \
C:/Work/prj/test_osg/av/DebugRenderer.cpp    \
C:/Work/prj/test_osg/av/Environment.cpp    \
C:/Work/prj/test_osg/av/EnvRenderer.cpp    \
C:/Work/prj/test_osg/av/Ephemeris.cpp    \
C:/Work/prj/test_osg/av/FogLayer.cpp    \
C:/Work/prj/test_osg/av/Grass.cpp    \
C:/Work/prj/test_osg/av/Grass2.cpp    \
C:/Work/prj/test_osg/av/LOD.cpp    \
C:/Work/prj/test_osg/av/Logo.cpp    \
C:/Work/prj/test_osg/av/Object.cpp    \
C:/Work/prj/test_osg/av/PreRender.cpp    \
C:/Work/prj/test_osg/av/Scene.cpp    \
C:/Work/prj/test_osg/av/scn_parser.cpp    \
C:/Work/prj/test_osg/av/Terrain.cpp    \
C:/Work/prj/test_osg/av/Utils.cpp    \
C:/Work/prj/test_osg/av/Visual.cpp    \
#lights    \
C:/Work/prj/test_osg/av/LightManager.cpp    \
C:/Work/prj/test_osg/av/Lights.cpp    \
#sky    \
C:/Work/prj/test_osg/av/avSky/CelestialBodies.cpp    \
C:/Work/prj/test_osg/av/avSky/DateTime.cpp    \
C:/Work/prj/test_osg/av/avSky/EphemerisEngine.cpp    \
C:/Work/prj/test_osg/av/avSky/EphemerisModel.cpp    \
C:/Work/prj/test_osg/av/avSky/Moon.cpp    \
C:/Work/prj/test_osg/av/avSky/Sky.cpp    \
C:/Work/prj/test_osg/av/avSky/SkyDome.cpp    \
C:/Work/prj/test_osg/av/avSky/StarField.cpp    \
#sm    \
C:/Work/prj/test_osg/av/avShadows/ShadowedScene.cpp    \
C:/Work/prj/test_osg/av/avShadows/ShadowMap.cpp    \
C:/Work/prj/test_osg/av/avShadows/ShadowSettings.cpp    \
C:/Work/prj/test_osg/av/avShadows/ShadowTechnique.cpp    \
C:/Work/prj/test_osg/av/avShadows/ViewDependentShadowMap.cpp \  
#shaders    \
C:/Work/prj/test_osg/av/shaders.cpp    \ 
#utils    \
C:/Work/prj/test_osg/core/config/config.cpp    \
C:/Work/prj/test_osg/utils/animation_handler.cpp    \
C:/Work/prj/test_osg/utils/empty_scene.cpp    \
C:/Work/prj/test_osg/utils/materials.cpp    \
C:/Work/prj/test_osg/utils/visitors/find_node_visitor.cpp    \
#mygui    \
#cegui    \
C:/Work/prj/test_osg/tests/cegui/CEGUIDrawable.cpp    \
C:/Work/prj/test_osg/tests/cegui/cegui_main.cpp    \
C:/Work/prj/test_osg/tests/common/CommonFunctions.cpp    \  
#spark    \
C:/Work/prj/test_osg/ext/spark/explosion_effect.cpp    \
C:/Work/prj/test_osg/ext/spark/fire_effect.cpp    \
C:/Work/prj/test_osg/ext/spark/osgspark.cpp    \
C:/Work/prj/test_osg/ext/spark/rain_effect.cpp    \
C:/Work/prj/test_osg/ext/spark/simple_effect.cpp    \
C:/Work/prj/test_osg/ext/spark/SparkDrawable.cpp    \
C:/Work/prj/test_osg/ext/spark/SparkUpdatingHandler.cpp    \
C:/Work/prj/test_osg/ext/spark/test_effect.cpp    \ 
#hmi    \
C:/Work/prj/test_osg/hmi/gui_vis/main_window_impl.cpp    \
C:/Work/prj/test_osg/hmi/gui_vis/menu_impl.cpp    \
C:/Work/prj/test_osg/hmi/gui_vis/panels/vis_settings_panel_impl.cpp    \
 