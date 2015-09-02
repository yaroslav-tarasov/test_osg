#include "stdafx.h"
#ifndef Q_MOC_RUN
#include <SPK.h>
#include <SPK_GL.h>
#endif
#include "SparkDrawable.h"

#define GET_TEXTURE_ID( name, var ) \
    GLuint var = 0; itr = textureIDMap.find(name); \
    if ( itr!=textureIDMap.end() ) var = itr->second;

// Gravity, the same for all particles
SPK::Vector3D gravity(0.0f,-0.9f,0.0f);

SPK::SPK_ID createTest( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight)
{
    SPK::Model* model;
    SPK::Emitter* emitter;
    SPK::Group* group;

    SPK::System* particleSystem = SPK::System::create();

    /* Yellow and red fireball */
    model = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
        SPK::FLAG_ALPHA,
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE
        );
    model->setParam(SPK::PARAM_ALPHA, 0.8f, 1.0f);
    model->setParam(SPK::PARAM_RED, 0.8f, 1.0f);
    model->setParam(SPK::PARAM_GREEN, 0.0f, 0.5f);
    model->setParam(SPK::PARAM_BLUE, 0.0f, 0.1f);
    model->setLifeTime(0.2f, 0.3f);

    // Emitter
    emitter = SPK::RandomEmitter::create();
    emitter->setZone(SPK::Point::create(SPK::Vector3D(0, 0, 0)));
    emitter->setFlow(-1);
    emitter->setTank(4000);
    emitter->setForce(20.0f, 40.0f);

    // Create group
    group = SPK::Group::create(model, 4000);
    group->addEmitter(emitter);
    group->setGravity(gravity);
    particleSystem->addGroup(group);

    /* Dustsplosion */
    model = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
        SPK::FLAG_ALPHA,
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE
        );
    model->setParam(SPK::PARAM_ALPHA, 0.2f, 0.0f);
    model->setParam(SPK::PARAM_RED, 0.6f, 0.8f);
    model->setParam(SPK::PARAM_GREEN, 0.6f, 0.8f);
    model->setParam(SPK::PARAM_BLUE, 0.6f, 0.8f);
    model->setLifeTime(1.2f, 8.0f);

    // Emitter
    emitter = SPK::NormalEmitter::create();
    emitter->setZone(SPK::Sphere::create(SPK::Vector3D(0, 0, 0), 2.0f));
    emitter->setFlow(-1);
    emitter->setTank(2000);
    emitter->setForce(3.0f, 5.0f);

    // Create group
    group = SPK::Group::create(model, 2000);
    group->addEmitter(emitter);
    group->setGravity(gravity);
    group->setFriction(1.0f);

    // System

    particleSystem->addGroup(group);
    return particleSystem->getSPKID();
}
