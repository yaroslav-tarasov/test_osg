TEMPLATE = app

DEV_ROOT = $$(SIMEX_DIR)

include($$DEV_ROOT/src/base.pri)

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



CONFIG += qt
QT += core gui widgets



#HEADERS += \
#    stdafx.h \
#    phys/RigidUpdater.h \
#    phys/rigid_body_info.h \
#    phys/phys_sys_common.h \
#    phys/GLDebugDrawer.h \
#    phys/BulletInterface.h \
#    phys/bullet_helpers.h \
#    av/Terrain.h \
#    utils/visitors/find_node_visitor.h \
#    creators.h \
#    tests/client.h \
#    utils/visitors/visitors.h \
#    utils/visitors/materials_visitor.h \
#    utils/visitors/info_visitor.h \
#    utils/visitors/find_tex_visitor.h \
#    utils/visitors/find_node_visitor.h \
#    utils/visitors/find_animation.h \
#    utils/visitors/ct_visitor.h \
#    av/Scene.h \
#    av/PreRender.h \
#    av/LOD.h \
#    av/FogLayer.h \
#    av/Ephemeris.h \
#    av/EnvRenderer.h \
#    av/CloudLayer.h

#SOURCES += \
#    test_osg.cpp \
#    objects/vehicle/vehicle_model_states.cpp \
#    objects/vehicle/vehicle_model.cpp \
    #static_convex.cpp \
#    shaders.cpp \
#    objects/nodes_manager/nodes_manager.cpp \
#    objects/nodes_manager/nodes_management.cpp \
#    objects/nodes_manager/node_impl.cpp \
#    materials.cpp \
#    dubins.cpp \
#    bada/bada_import.cpp \
#    animation_handler.cpp \
#    objects/aircraft/aircraft_visual.cpp \
#    objects/aircraft/aircraft_shassis_impl.cpp \
#    objects/aircraft/aircraft_model.cpp \
#    phys/RigidUpdater.cpp \
#    phys/GLDebugDrawer.cpp \
#    phys/BulletInterface.cpp \
#    phys/ray_cast_vehicle.cpp \
#    phys/phys_aircraft.cpp \
#    phys/bvh_static_mesh.cpp \
#    phys/aircraft_phys.cpp \
#    nfi/lib_loader.cpp \
#    sm/ViewDependentShadowMap.cpp \
#    sm/ShadowTechnique.cpp \
#    sm/ShadowSettings.cpp \
#    sm/ShadowMap.cpp \
#    sm/ShadowedScene.cpp \
#    av/Terrain.cpp \
#    av/Scene.cpp \
#    av/PreRender.cpp \
#    av/Object.cpp \
#    av/LOD.cpp \
#    av/FogLayer.cpp \
#    av/Ephemeris.cpp \
#    av/EnvRenderer.cpp \
#    av/CloudLayer.cpp \
#    utils/visitors/find_node_visitor.cpp \
#    creators.cpp \
#    ext/spark/SparkUpdatingHandler.cpp \
#    ext/spark/SparkDrawable.cpp \
#    ext/spark/simple_effect.cpp \
#    ext/spark/rain_effect.cpp \
#    ext/spark/fire_effect.cpp \
#    ext/spark/explosion_effect.cpp \
#    ext/spark/osgspark.cpp \
#    main_scene2.cpp \
#    tests/test_network.cpp \
#    tests/sync_qt.cpp \
#    utils/visitors/find_node_visitor.cpp


