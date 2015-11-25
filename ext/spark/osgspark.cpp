#include "stdafx.h"

#include "osgspark.h"
#include <GL/glut.h>
#ifndef Q_MOC_RUN
#include <SPK.h>
#include <SPK_GL.h>
#endif
#include "SparkDrawable.h"
#include "SparkUpdatingHandler.h"
#include "SmokeNode.h"

#ifndef _DEBUG
#pragma comment(lib, "SPARK_GL.lib")
#pragma comment(lib, "SPARK.lib")
#else 
#pragma comment(lib, "SPARK_GL_debug.lib")
#pragma comment(lib, "SPARK_debug.lib")
#endif

//
//     C-like frdws
//  
extern SPK::SPK_ID createSimpleSystem ( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createSmoke        ( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createExplosion    ( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createFire         ( const SparkDrawable::TextureIDMap&, int, int , float);
extern SPK::SPK_ID createRain         ( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createTest         ( const SparkDrawable::TextureIDMap&, int, int );
extern SPK::SPK_ID createFireSmoke    ( const SparkDrawable::TextureIDMap&, int, int , float);
extern SPK::SPK_ID createSomething    ( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight );

namespace {
osg::AnimationPath* createAnimationPath( float radius, float time )
{
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
    path->setLoopMode( osg::AnimationPath::LOOP );
    
    unsigned int numSamples = 32;
    float delta_yaw = 2.0f * osg::PI/((float)numSamples - 1.0f);
    float delta_time = time / (float)numSamples;
    for ( unsigned int i=0; i<numSamples; ++i )
    {
        float yaw = delta_yaw * (float)i;
        osg::Vec3 pos( sinf(yaw)*radius, cosf(yaw)*radius, 0.0f );
        osg::Quat rot( -yaw, osg::Z_AXIS );
        path->insert( delta_time * (float)i, osg::AnimationPath::ControlPoint(pos, rot) );
    }
    return path.release();    
}

struct fire_creator
{
    fire_creator(float sc = 1.0f)
    {
        scale_coeff(sc);
    }

    static SPK::SPK_ID createFire( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
    {
        return ::createFire(textureIDMap,screenWidth,screenHeight,scale_coeff());
    }

    static SPK::SPK_ID createFireSmoke( const SparkDrawable::TextureIDMap& textureIDMap, int screenWidth, int screenHeight )
    {
        return ::createFireSmoke(textureIDMap,screenWidth,screenHeight,scale_coeff());
    }

private:
    static inline float scale_coeff(float sc = 1.0f)
    {
        static float scale_coeff_ = sc;
        return scale_coeff_;
    }
};    

};

namespace spark
{
   
    void init()
    {
        SPK::randomSeed = static_cast<unsigned int>( time(NULL) );
        SPK::System::setClampStep( true, 0.1f );
        SPK::System::useAdaptiveStep( 0.001f, 0.01f );
    }

    FIXME("Возвращаем handler каждый раз, а нужно только один раз добавить к вьюверу");

    spark_pair_t create(spark_t effectType,osg::Transform* model)
    {
        static int count = 0; 
        osg::ref_ptr<SparkDrawable> spark = new SparkDrawable;
		osg::ref_ptr<osg::Geode>    geode = nullptr;

        // bool trackingModel = false;
        fire_creator fc(2.0);
        switch ( effectType )
        {
        case EXPLOSION:  // Explosion
            spark->setBaseSystemCreator( &createExplosion );
            spark->addParticleSystem();
            spark->setSortParticles( true );
            spark->addImage( "explosion", osgDB::readImageFile("data/explosion.bmp"), GL_ALPHA );
            spark->addImage( "flash"    , osgDB::readImageFile("data/flash.bmp")    , GL_RGB   );
            spark->addImage( "spark1"   , osgDB::readImageFile("data/spark1.bmp")   , GL_RGB   );
            spark->addImage( "spark2"   , osgDB::readImageFile("data/point.bmp")    , GL_ALPHA );
            spark->addImage( "wave"     , osgDB::readImageFile("data/wave.bmp")     , GL_RGBA  );
			geode = new osg::Geode;
            geode->setName("fxExplosion");
            break;
        case FIRE:  // Fire
            spark->setBaseSystemCreator( &fc.createFireSmoke );
            spark->addParticleSystem();
            spark->addImage( "fire"     , osgDB::readImageFile("data/fire2.bmp")    , GL_ALPHA );
            spark->addImage( "explosion", osgDB::readImageFile("data/explosion.bmp"), GL_ALPHA );
            spark->addImage( "smoke"    , osgDB::readImageFile("data/smoke_black.png"), GL_RGBA );
            geode = new SmokeNode;
            dynamic_cast<SmokeNode*>(geode.get())->setGravity(osg::Vec3f(1.0,1.0,0.05));
            geode->setName("fxFire");
            break;
        case RAIN:  // Rain
            spark->setBaseSystemCreator( &createRain, true );  // Must use the proto type directly
            spark->addImage( "waterdrops", osgDB::readImageFile("data/waterdrops.bmp"), GL_ALPHA );
			geode = new osg::Geode;
            geode->setName("fxRain");
            break;
        case SMOKE:  // Smoke
            spark->setBaseSystemCreator( &createSmoke );
            spark->addParticleSystem();
            spark->addImage( "smoke", osgDB::readImageFile("data/smoke_black.png"), GL_RGBA );
            //spark->addImage( "smoke", osgDB::readImageFile("data/fire2.bmp"), GL_RGBA );
			geode = new SmokeNode;
			dynamic_cast<SmokeNode*>(geode.get())->setGravity(osg::Vec3f(1.0,1.0,0.05));
            geode->setName("fxSmoke");
            ///trackingModel = true;
            break;
        case TEST:
            spark->setBaseSystemCreator( &createTest );
            spark->addParticleSystem();
			geode = new osg::Geode;
            geode->setName("fxTest");
            break;
        case SOMETHING:
            spark->setBaseSystemCreator( &createSomething);  // Must use the proto type directly
            spark->addImage( "waterdrops", osgDB::readImageFile("data/waterdrops.bmp"), GL_ALPHA );
            geode = new osg::Geode;
            geode->setName("fxRain");
            
            break;
        default:  // Simple
            spark->setBaseSystemCreator( &createSimpleSystem );
            spark->addParticleSystem();
            spark->addImage( "flare", osgDB::readImageFile("data/flare.bmp"), GL_ALPHA );
			geode = new osg::Geode;
            geode->setName("fxFlare");
            break;
        }

        geode->addDrawable( spark.get() );
        geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		geode->getOrCreateStateSet()->setNestRenderBins(false);

        static osg::ref_ptr<SparkUpdatingHandler> handler = new SparkUpdatingHandler;
        handler->addSpark( spark.get() );
        
        if ( /*trackingModel*/model != nullptr )
        {
            handler->setTrackee( count, model );
        }
        
        count++;

        return spark_pair_t(geode.release(),handler.get());
    }

		
}


int main_spark( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    bool trackingModel = false;
    int effectType = 0;
    
    if ( arguments.read("--simple") ) effectType = 0;
    else if ( arguments.read("--explosion") ) effectType = 1;
    else if ( arguments.read("--fire") ) effectType = 2;
    else if ( arguments.read("--rain") ) effectType = 3;
    else if ( arguments.read("--smoke") ) effectType = 4;
    
    effectType = 3;

    spark::init();

    osg::ref_ptr<SparkDrawable> spark = new SparkDrawable;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	switch ( effectType )
    {
    case 1:  // Explosion
        spark->setBaseSystemCreator( &createExplosion );
        spark->addParticleSystem();
        spark->setSortParticles( true );
        spark->addImage( "explosion", osgDB::readImageFile("data/explosion.bmp"), GL_ALPHA );
        spark->addImage( "flash", osgDB::readImageFile("data/flash.bmp"), GL_RGB );
        spark->addImage( "spark1", osgDB::readImageFile("data/spark1.bmp"), GL_RGB );
        spark->addImage( "spark2", osgDB::readImageFile("data/point.bmp"), GL_ALPHA );
        spark->addImage( "wave", osgDB::readImageFile("data/wave.bmp"), GL_RGBA );
        break;
    case 2:  // Fire
        spark->setBaseSystemCreator( /*&createFire*/&fire_creator(2).createFire );
        spark->addParticleSystem();
        spark->addImage( "fire", osgDB::readImageFile("data/fire2.bmp"), GL_ALPHA );
        spark->addImage( "explosion", osgDB::readImageFile("data/explosion.bmp"), GL_ALPHA );
        break;
    case 3:  // Rain
        spark->setBaseSystemCreator( &createRain, true );  // Must use the proto type directly
        spark->addImage( "waterdrops", osgDB::readImageFile("data/waterdrops.bmp"), GL_ALPHA );
		geode = new osg::Geode;
        break;
    case 4:  // Smoke
        spark->setBaseSystemCreator( &createSmoke );
        spark->addParticleSystem();
        spark->addImage( "smoke", osgDB::readImageFile("data/smoke.png"), GL_RGBA );
        trackingModel = true;
        break;
    default:  // Simple
        spark->setBaseSystemCreator( &createSimpleSystem );
        spark->addParticleSystem();
        spark->addImage( "flare", osgDB::readImageFile("data/flare.bmp"), GL_ALPHA );
        break;
    }
    
    geode->addDrawable( spark.get() );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    
    osg::ref_ptr<SparkUpdatingHandler> handler = new SparkUpdatingHandler;
    handler->addSpark( spark.get() );
    
    osg::ref_ptr<osg::MatrixTransform> model = new osg::MatrixTransform;
    model->addChild( osgDB::readNodeFile("glider.osg") );
    if ( trackingModel )
    {
        handler->setTrackee( 0, model.get() );
        
        osg::ref_ptr<osg::AnimationPathCallback> apcb = new osg::AnimationPathCallback;
        apcb->setAnimationPath( createAnimationPath(4.0f, 6.0f) );
        model->setUpdateCallback( apcb.get() );
    }
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( geode.get() );
    root->addChild( model.get() );
    
    osgViewer::Viewer viewer(arguments);
    viewer.getCamera()->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( handler.get() );
    return viewer.run();
}


AUTO_REG(main_spark)
