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

DEFINES += VISUAL_EXPORTS
# For spark only
DEFINES += QT_NO_EMIT

PRECOMPILED_HEADER = precompiled.h

HEADERS += \
    stdafx.h 


SOURCES += \   
#av    \
$$SOLUTION_ROOT/av/CloudLayer.cpp    \
$$SOLUTION_ROOT/av/EnvRenderer.cpp    \
$$SOLUTION_ROOT/av/Ephemeris.cpp    \
$$SOLUTION_ROOT/av/FogLayer.cpp    \
$$SOLUTION_ROOT/av/Grass.cpp    \
$$SOLUTION_ROOT/av/Grass2.cpp    \
$$SOLUTION_ROOT/av/Object.cpp    \
$$SOLUTION_ROOT/av/PreRender.cpp    \
$$SOLUTION_ROOT/av/scn_parser.cpp    \
$$SOLUTION_ROOT/av/Terrain.cpp    \
$$SOLUTION_ROOT/av/Visual.cpp    \
$$SOLUTION_ROOT/av/LightningLayer.cpp \
#weather                         \
$$SOLUTION_ROOT/av/avWeather/Cloud.cpp             \  
$$SOLUTION_ROOT/av/avWeather/FogBank.cpp           \
$$SOLUTION_ROOT/av/avWeather/LocalWeather.cpp      \
$$SOLUTION_ROOT/av/avWeather/PrecipitationBase.cpp \
$$SOLUTION_ROOT/av/avWeather/Weather.cpp           \
#sm        \
$$SOLUTION_ROOT/av/avShadows/ParallelSplitShadowMap.cpp \
$$SOLUTION_ROOT/av/avShadows/ShadowedScene.cpp \
$$SOLUTION_ROOT/av/avShadows/ShadowMap.cpp      \
$$SOLUTION_ROOT/av/avShadows/ShadowSettings.cpp      \
$$SOLUTION_ROOT/av/avShadows/ShadowTechnique.cpp      \
$$SOLUTION_ROOT/av/avShadows/ViewDependentShadowMap.cpp \
#sky    \
$$SOLUTION_ROOT/av/avSky/CelestialBodies.cpp    \
$$SOLUTION_ROOT/av/avSky/DateTime.cpp    \
$$SOLUTION_ROOT/av/avSky/EphemerisEngine.cpp    \
$$SOLUTION_ROOT/av/avSky/EphemerisModel.cpp    \
$$SOLUTION_ROOT/av/avSky/Moon.cpp    \
$$SOLUTION_ROOT/av/avSky/Sky.cpp    \
$$SOLUTION_ROOT/av/avSky/SkyDome.cpp    \
$$SOLUTION_ROOT/av/avSky/StarField.cpp    \
#scene     \
$$SOLUTION_ROOT/av/avScene/Scene.cpp      \
$$SOLUTION_ROOT/av/avScene/ScreenTexture.cpp      \
$$SOLUTION_ROOT/av/avScene/ScreenTextureManager.cpp      \
#lights    \
$$SOLUTION_ROOT/av/avLights/NavAid.cpp      \
$$SOLUTION_ROOT/av/avLights/Lights.cpp      \
$$SOLUTION_ROOT/av/avLights/LightMaps.cpp      \
$$SOLUTION_ROOT/av/avLights/LightManager.cpp      \
#core    \
$$SOLUTION_ROOT/av/avCore/Utils.cpp      \
$$SOLUTION_ROOT/av/avCore/Timer.cpp      \
$$SOLUTION_ROOT/av/avCore/Logo.cpp      \
$$SOLUTION_ROOT/av/avCore/LOD.cpp      \
$$SOLUTION_ROOT/av/avCore/Environment.cpp      \
$$SOLUTION_ROOT/av/avCore/DebugRenderer.cpp      \
$$SOLUTION_ROOT/av/avCore/Database.cpp      \
#animation   \
$$SOLUTION_ROOT/av/avAnimation/XFileParser.cpp      \
$$SOLUTION_ROOT/av/avAnimation/Bone.cpp      \
$$SOLUTION_ROOT/av/avAnimation/AnimTest.cpp      \
$$SOLUTION_ROOT/av/avAnimation/AnimationObject.cpp      \
$$SOLUTION_ROOT/av/avAnimation/AnimationPrototype.cpp      \
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
$$SOLUTION_ROOT/ext/spark/SmokeNode.cpp  \
#hmi    \
$$SOLUTION_ROOT/hmi/gui_vis/main_window_impl.cpp    \
$$SOLUTION_ROOT/hmi/gui_vis/menu_impl.cpp    \
$$SOLUTION_ROOT/hmi/gui_vis/panels/vis_settings_panel_impl.cpp    \
 