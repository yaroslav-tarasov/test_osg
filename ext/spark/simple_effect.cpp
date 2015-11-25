#include "stdafx.h"

#ifndef Q_MOC_RUN
#include <SPK.h>
#include <SPK_GL.h>
#endif
#include "SparkDrawable.h"



#define GET_TEXTURE_ID( name, var ) \
    GLuint var = 0; itr = textureIDMap.find(name); \
    if ( itr!=textureIDMap.end() ) var = itr->second;

SPK::SPK_ID createSimpleSystem( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "flare", flareTexID );
    
    // Create the model
    SPK::Model* model = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
        SPK::FLAG_ALPHA, SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE );
    model->setParam( SPK::PARAM_ALPHA, 1.0f, 0.0f );
    model->setLifeTime( 1.0f, 2.0f );
    
    // Create the renderer
    SPK::GL::GLRenderer* renderer = NULL;
    if ( SPK::GL::GLPointRenderer::loadGLExtPointSprite() &&
         SPK::GL::GLPointRenderer::loadGLExtPointParameter() )
    {
        SPK::GL::GLPointRenderer* pointRenderer = SPK::GL::GLPointRenderer::create();
        pointRenderer->setType( SPK::POINT_SPRITE );
        pointRenderer->enableWorldSize( true );
        SPK::GL::GLPointRenderer::setPixelPerUnit( 45.0f * 3.14159f / 180.0f, screenHeight );
        pointRenderer->setSize( 0.1f );
        pointRenderer->setTexture( flareTexID );
        renderer = pointRenderer;
    }
    else
    {
        SPK::GL::GLQuadRenderer* quadRenderer = SPK::GL::GLQuadRenderer::create();
        quadRenderer->setTexturingMode( SPK::TEXTURE_2D );
        quadRenderer->setScale( 0.1f, 0.1f );
        quadRenderer->setTexture( flareTexID );
        renderer = quadRenderer;
    }
    
    renderer->enableBlending( true );
    renderer->setBlendingFunctions( GL_SRC_ALPHA, GL_ONE );
    renderer->setTextureBlending( GL_MODULATE );
    renderer->enableRenderingHint( SPK::DEPTH_TEST, false );
    
    // Create the zone
    SPK::Point* source = SPK::Point::create();
    
    // Creates the emitter
    SPK::RandomEmitter* emitter = SPK::RandomEmitter::create();
    emitter->setZone( source );
    emitter->setForce( 2.8f, 3.2f );
    emitter->setTank( 500 );
    emitter->setFlow( -1.0f );
    
    // Creates the Group
    SPK::Group* group = SPK::Group::create( model, 500 );
    group->addEmitter( emitter );
    group->setRenderer( renderer );
    group->setGravity( SPK::Vector3D(0.0f, 0.0f, -1.0f) );
    group->setFriction( 2.0f );
    group->enableAABBComputing( true );
    
    // Creates the System
    SPK::System* system = SPK::System::create();
    system->addGroup( group );
    system->enableAABBComputing( true );
    
    // Creates the base and gets a pointer to the base
    model->setShared( true );
    renderer->setShared( true );
    return system->getSPKID();
}

SPK::SPK_ID createSmoke( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    const float scale_coeff = 20;  
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "smoke", textureParticle );
    
    SPK::GL::GLQuadRenderer* particleRenderer = SPK::GL::GLQuadRenderer::create();
    particleRenderer->setTexturingMode( SPK::TEXTURE_2D );
    particleRenderer->setAtlasDimensions( 2, 2 );
    particleRenderer->setTexture( textureParticle );
    particleRenderer->setTextureBlending( GL_MODULATE );
    particleRenderer->setScale( 0.05f*scale_coeff, 0.05f*scale_coeff );
    particleRenderer->setBlending( /*SPK::BLENDING_ADD*/SPK::BLENDING_ALPHA );
    particleRenderer->enableRenderingHint( SPK::DEPTH_WRITE, false );

               
#if 0 // Ёффект рваной горелой бумаги
    particleRenderer->enableRenderingHint( SPK::DEPTH_WRITE, true );
    particleRenderer->enableRenderingHint(SPK::ALPHA_TEST,true);
    particleRenderer->setAlphaTestThreshold(0.8f);
#endif

    // Model
    SPK::Model* particleModel = SPK::Model::create(
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_SIZE | SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE,
        SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_SIZE | SPK::FLAG_ALPHA,
        SPK::FLAG_SIZE | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE /*,
        SPK::FLAG_ALPHA*/);
    
    particleModel->setParam(SPK::PARAM_RED,/*0.3f*/0.5,0.2f);
    particleModel->setParam(SPK::PARAM_GREEN,0.25f,0.2f);
    particleModel->setParam(SPK::PARAM_BLUE,0.2f);

    particleModel->setParam( SPK::PARAM_SIZE, 0.5f, 1.0f, 10.0f, 20.0f );
    particleModel->setParam( SPK::PARAM_ALPHA, 1.0f, 0.0f );
    particleModel->setParam( SPK::PARAM_ANGLE, 0.0f, 2.0f * osg::PI );
    particleModel->setParam( SPK::PARAM_TEXTURE_INDEX, 0.0f, 4.0f );
    particleModel->setLifeTime( /*2.0*/5.f, 15.0f );
   
    //SPK::Interpolator* interpolator = particleModel->getInterpolator(SPK::PARAM_ALPHA);
    //interpolator->addEntry(0.0f,0.0f);
    //interpolator->addEntry(0.2f,0.2f);
    //interpolator->addEntry(1.0f,0.0f);

    // Emitter
    SPK::SphericEmitter* particleEmitter = SPK::SphericEmitter::create(
        SPK::Vector3D(/*-1.0f*/0.0f, 0.0f, 1.0f), 0.0f, 0.1f * osg::PI );
//    particleEmitter->setZone( SPK::Point::create(SPK::Vector3D(0.0f, 0.015f, 0.0f)) );
    particleEmitter->setFlow( 250.0 );
    particleEmitter->setForce( 1.5f, 1.5f );
    
    // Group
    SPK::Group* particleGroup = SPK::Group::create( particleModel, 1500 );
    particleGroup->addEmitter( particleEmitter );
    particleGroup->setRenderer( particleRenderer );
    particleGroup->setGravity( SPK::Vector3D(0.0f, 0.0f, 0.05f) );
    particleGroup->enableAABBComputing( true );
    
    SPK::System* particleSystem = SPK::System::create();
    particleSystem->addGroup( particleGroup );
    particleSystem->enableAABBComputing( true );
    return particleSystem->getSPKID();
}

using namespace SPK;
using namespace SPK::GL;

// Call back function to transform water particles that touches the water into splash particles
bool splash(Particle& particle,float deltaTime)
{
    if (particle.position().y < 0.1f)
    {
        if (particle.velocity().y > -0.5f)
            return true;

        particle.position().y = 0.1f;
        particle.position().x += random(0.0f,0.2f) - 0.1f;
        particle.position().z += random(0.0f,0.2f) - 0.1f;

        particle.velocity().set(0,-random(0.1f,0.4f) * particle.velocity().y,0);

        particle.setParamCurrentValue(PARAM_ALPHA,0.4f);
        particle.setParamCurrentValue(PARAM_SIZE,0.0f);

        particle.setLifeLeft(0.5f);
    }

    return false;
}


SPK::SPK_ID createSomething( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
{
    Group* particleGroup = NULL;
    System* particleSystem = NULL;

    const float scale_coeff = 20;  
    SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "waterdrops", textureSplash );


    // Inits Particle Engine
    Vector3D gravity(0.0f,-2.2f,0.0f);

    // Renderer
    GLPointRenderer* basicRenderer = GLPointRenderer::create();

    GLQuadRenderer* particleRenderer = GLQuadRenderer::create();
    particleRenderer->setScale(0.06f,0.06f);
    particleRenderer->setTexturingMode(TEXTURE_2D);
    particleRenderer->setTexture(textureSplash);
    particleRenderer->setBlending(BLENDING_ALPHA);
    particleRenderer->enableRenderingHint(DEPTH_WRITE,false);

    // Model
    Model* particleModel = Model::create(FLAG_ALPHA | FLAG_SIZE | FLAG_ANGLE,
        FLAG_ALPHA | FLAG_SIZE | FLAG_ANGLE,
        FLAG_SIZE | FLAG_ANGLE);

    particleModel->setLifeTime(1.6f,2.2f);
    particleModel->setParam(PARAM_ALPHA,0.2f,0.0f);
    particleModel->setParam(PARAM_SIZE,1.0f,1.0f,2.0f,8.0f);
    particleModel->setParam(PARAM_ANGLE,0.0f,4.0f * osg::PI,0.0f,4.0f * osg::PI);

    // Emitters
    const int NB_EMITTERS = 13;

    Point* emitterZone[NB_EMITTERS];
    emitterZone[0] = Point::create(Vector3D(0.0f,0.1f,0.0f));

    emitterZone[1] = Point::create(Vector3D(0.0f,0.1f,0.0f));
    emitterZone[2] = Point::create(Vector3D(0.0f,0.1f,0.0f));
    emitterZone[3] = Point::create(Vector3D(0.0f,0.1f,0.0f));
    emitterZone[4] = Point::create(Vector3D(0.0f,0.1f,0.0f));

    emitterZone[5] = Point::create(Vector3D(-1.6f,0.1f,-1.6f));
    emitterZone[6] = Point::create(Vector3D(1.6f,0.1f,1.6f));
    emitterZone[7] = Point::create(Vector3D(1.6f,0.1f,-1.6f));
    emitterZone[8] = Point::create(Vector3D(-1.6f,0.1f,1.6f));
    emitterZone[9] = Point::create(Vector3D(-2.26f,0.1f,0.0f));
    emitterZone[10] = Point::create(Vector3D(2.26f,0.1f,0.0f));
    emitterZone[11] = Point::create(Vector3D(0.0f,0.1f,-2.26f));
    emitterZone[12] = Point::create(Vector3D(0.0f,0.1f,2.26f));

    StraightEmitter* particleEmitter[NB_EMITTERS];
    particleEmitter[0] = StraightEmitter::create(Vector3D(0.0f,1.0f,0.0f));

    particleEmitter[1] = StraightEmitter::create(Vector3D(1.0f,3.0f,1.0f));
    particleEmitter[2] = StraightEmitter::create(Vector3D(-1.0f,3.0f,-1.0f));
    particleEmitter[3] = StraightEmitter::create(Vector3D(-1.0f,3.0f,1.0f));
    particleEmitter[4] = StraightEmitter::create(Vector3D(1.0f,3.0f,-1.0f));

    particleEmitter[5] = StraightEmitter::create(Vector3D(1.0f,2.0f,1.0f));
    particleEmitter[6] = StraightEmitter::create(Vector3D(-1.0f,2.0f,-1.0f));
    particleEmitter[7] = StraightEmitter::create(Vector3D(-1.0f,2.0f,1.0f));
    particleEmitter[8] = StraightEmitter::create(Vector3D(1.0f,2.0f,-1.0f));
    particleEmitter[9] = StraightEmitter::create(Vector3D(1.41f,2.0f,0.0f));
    particleEmitter[10] = StraightEmitter::create(Vector3D(-1.41f,2.0f,0.0f));
    particleEmitter[11] = StraightEmitter::create(Vector3D(0.0f,2.0f,1.41f));
    particleEmitter[12] = StraightEmitter::create(Vector3D(0.0f,2.0f,-1.41f));

    float flow[NB_EMITTERS] =
    {
        500.0f,

        600.0f,
        600.0f,
        600.0f,
        600.0f,

        900.0f,
        900.0f,
        900.0f,
        900.0f,
        900.0f,
        900.0f,
        900.0f,
        900.0f,
    };

    float flowLow[NB_EMITTERS] =
    {
        150.0f,

        200.0f,
        200.0f,
        200.0f,
        200.0f,

        250.0f,
        250.0f,
        250.0f,
        250.0f,
        250.0f,
        250.0f,
        250.0f,
        250.0f,
    };

    for (int i = 0; i < NB_EMITTERS; ++i)
    {
        particleEmitter[i]->setZone(emitterZone[i]);
        particleEmitter[i]->setFlow(flow[i]);
        particleEmitter[i]->setForce(2.5f,4.0f);
    }
    particleEmitter[0]->setForce(3.0f,3.5f);

    // Group
    particleGroup = Group::create(particleModel,20000);
    particleGroup->setRenderer(particleRenderer);
    for (int i = 0; i < NB_EMITTERS; ++i)
        particleGroup->addEmitter(particleEmitter[i]);
    particleGroup->setCustomUpdate(&splash);
    particleGroup->setGravity(gravity);
    particleGroup->setFriction(0.7f);
    particleGroup->enableAABBComputing( true );

    // System
    particleSystem = System::create();
    particleSystem->addGroup(particleGroup);
    particleSystem->enableAABBComputing( true );

    return particleSystem->getSPKID();
}