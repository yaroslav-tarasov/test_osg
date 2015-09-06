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
$$SOLUTION_ROOT/av/CloudLayer.cpp    \
$$SOLUTION_ROOT/av/Database.cpp    \
$$SOLUTION_ROOT/av/DebugRenderer.cpp    \
$$SOLUTION_ROOT/av/Environment.cpp    \
$$SOLUTION_ROOT/av/EnvRenderer.cpp    \
$$SOLUTION_ROOT/av/Ephemeris.cpp    \
$$SOLUTION_ROOT/av/FogLayer.cpp    \
$$SOLUTION_ROOT/av/Grass.cpp    \
$$SOLUTION_ROOT/av/Grass2.cpp    \
$$SOLUTION_ROOT/av/LOD.cpp    \
$$SOLUTION_ROOT/av/Logo.cpp    \
$$SOLUTION_ROOT/av/Object.cpp    \
$$SOLUTION_ROOT/av/PreRender.cpp    \
$$SOLUTION_ROOT/av/Scene.cpp    \
$$SOLUTION_ROOT/av/scn_parser.cpp    \
$$SOLUTION_ROOT/av/Terrain.cpp    \
$$SOLUTION_ROOT/av/Utils.cpp    \
$$SOLUTION_ROOT/av/Visual.cpp    \
#lights    \
$$SOLUTION_ROOT/av/LightManager.cpp    \
$$SOLUTION_ROOT/av/Lights.cpp    \
#sky    \
$$SOLUTION_ROOT/av/avSky/CelestialBodies.cpp    \
$$SOLUTION_ROOT/av/avSky/DateTime.cpp    \
$$SOLUTION_ROOT/av/avSky/EphemerisEngine.cpp    \
$$SOLUTION_ROOT/av/avSky/EphemerisModel.cpp    \
$$SOLUTION_ROOT/av/avSky/Moon.cpp    \
$$SOLUTION_ROOT/av/avSky/Sky.cpp    \
$$SOLUTION_ROOT/av/avSky/SkyDome.cpp    \
$$SOLUTION_ROOT/av/avSky/StarField.cpp    \
#sm    \
$$SOLUTION_ROOT/av/avShadows/ShadowedScene.cpp    \
$$SOLUTION_ROOT/av/avShadows/ShadowMap.cpp    \
$$SOLUTION_ROOT/av/avShadows/ShadowSettings.cpp    \
$$SOLUTION_ROOT/av/avShadows/ShadowTechnique.cpp    \
$$SOLUTION_ROOT/av/avShadows/ViewDependentShadowMap.cpp \  
#shaders    \
$$SOLUTION_ROOT/av/shaders.cpp    \ 
#utils    \
$$SOLUTION_ROOT/core/config/config.cpp    \
$$SOLUTION_ROOT/utils/animation_handler.cpp    \
$$SOLUTION_ROOT/utils/empty_scene.cpp    \
$$SOLUTION_ROOT/utils/materials.cpp    \
$$SOLUTION_ROOT/utils/visitors/find_node_visitor.cpp    \
#mygui    \
#cegui    \
$$SOLUTION_ROOT/tests/cegui/CEGUIDrawable.cpp    \
$$SOLUTION_ROOT/tests/cegui/cegui_main.cpp    \
$$SOLUTION_ROOT/tests/common/CommonFunctions.cpp    \  
#spark    \
$$SOLUTION_ROOT/ext/spark/explosion_effect.cpp    \
$$SOLUTION_ROOT/ext/spark/fire_effect.cpp    \
$$SOLUTION_ROOT/ext/spark/osgspark.cpp    \
$$SOLUTION_ROOT/ext/spark/rain_effect.cpp    \
$$SOLUTION_ROOT/ext/spark/simple_effect.cpp    \
$$SOLUTION_ROOT/ext/spark/SparkDrawable.cpp    \
$$SOLUTION_ROOT/ext/spark/SparkUpdatingHandler.cpp    \
$$SOLUTION_ROOT/ext/spark/test_effect.cpp    \ 
#hmi    \
$$SOLUTION_ROOT/hmi/gui_vis/main_window_impl.cpp    \
$$SOLUTION_ROOT/hmi/gui_vis/menu_impl.cpp    \
$$SOLUTION_ROOT/hmi/gui_vis/panels/vis_settings_panel_impl.cpp    \
 