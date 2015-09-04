// test_osg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "av/precompiled.h"
#include "creators.h"
#include "animation_handler.h"
#include "visitors/info_visitor.h"

#include "SkyBox.h"
#include "av/FogLayer.h"
#include "av/CloudLayer.h"
#include "av/PreRender.h"
#include "av/EnvRenderer.h"


#include "high_res_timer.h"
#include "phys/BulletInterface.h"
#include "phys/RigidUpdater.h"

#include <osgEphemeris/EphemerisModel.h>


#include "shadow_map.h"
#include "teapot.h"

//
//  ext
//
#include "spark/osgspark.h"

#if defined (DEVELOP_SHADOWS) || defined(TEST_SHADOWS_FROM_OSG)
#if !defined(TEST_SHADOWS_FROM_OSG)
#define TEST_SHADOWS_2
#endif
#else
#define TEST_EPHEMERIS
#define TEST_PRECIP
#define TEST_SV_FOG
#define TEST_SV_CLOUD
#endif

#define TEST_EPHEMERIS
#define TEST_PRECIP
#define TEST_SV_FOG
#define TEST_SV_CLOUD

// #define TEST_OSG_FOG
// #define TEST_NODE_TRACKER
// #define TEST_SKYBOX
// #define TEST_CAMERA
// #define TEST_SHADOWS

////You can compute a vertex in the absolute coordinate frame by using the
////    osg::computeLocalToWorld() function:
//osg::Vec3 posInWorld = node->getBound().center() *
//    osg::computeLocalToWorld(node->getParentalNodePaths()[0]);

osg::Matrix computeTargetToWorldMatrix( osg::Node* node ) // const
{
    osg::Matrix l2w;
    if ( node && node->getNumParents()>0 )
    {
        osg::Group* parent = node->getParent(0);
        l2w = osg::computeLocalToWorld( parent->
            getParentalNodePaths()[0] );
    }
    return l2w;
}




void AddLight( osg::ref_ptr<osg::MatrixTransform> rootnode ) 
{

    
    osg::Node* light0 = effects::createLightSource(
        0, osg::Vec3(0.0f,0.0f,1000.0f), osg::Vec4(0.0f,0.0f,0.0f,0.0f) );

    osg::Node* light1 = effects::createLightSource(
        1, osg::Vec3(/*0.0f,-20.0f,0.0f*/0.0f,0.0f,1000.0f), osg::Vec4(1.5f,1.5f,1.5f,1.0f)
        );


    rootnode->getOrCreateStateSet()->setMode( GL_LIGHT0,
        osg::StateAttribute::ON );
    rootnode->getOrCreateStateSet()->setMode( GL_LIGHT1,
        osg::StateAttribute::ON );
    rootnode->addChild( light0 );
    rootnode->addChild( light1 );
}

class circleAimlessly : public osg::NodeCallback 
{
public:
    circleAimlessly(float angle=0.f): _angle(angle) {}
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
        if( tx != NULL)
        {
            _angle += osg::PI/180.0;
            tx->setMatrix( osg::Matrix::translate( 30.0, 0.0, 5.0) * 
                osg::Matrix::rotate( _angle, 0, 0, 1 ) );
        }
        traverse(node, nv);
    }
private:
    float _angle;
};

class MyGustCallback : public osg::NodeCallback
{

    public:

        MyGustCallback() {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osgParticle::PrecipitationEffect* pe = dynamic_cast<osgParticle::PrecipitationEffect*>(node);
            
            float value = sin(nv->getFrameStamp()->getSimulationTime());
            if (value<-0.5)
            {
                pe->snow(1.0);
            }
            else
            {
                pe->rain(0.5);
            }
        
            traverse(node, nv);
        }
};

class EphemerisDataUpdateCallback : virtual public osgEphemeris::EphemerisUpdateCallback
{
public:
    EphemerisDataUpdateCallback(osgEphemeris::EphemerisModel *ephem ,avSky::FogLayer* fogLayer,avSky::CloudsLayer* skyClouds) 
        : EphemerisUpdateCallback( "EphemerisDataUpdateCallback" )
        , _ephem      (ephem)
        , _fogLayer   (fogLayer)
        , _skyClouds  (skyClouds)
        , _handler    (new handler(_skyClouds.get()))
    {}

    void operator()( osgEphemeris::EphemerisData *data )
    {
        auto sls = _ephem->getSunLightSource()->getLight();
        
        if(!_fogLayer || !_skyClouds)
            return;

        //time_t seconds = time(0L);
        //struct tm *_tm = localtime(&seconds);

        //data->dateTime.setYear( _tm->tm_year + 1900 ); // DateTime uses _actual_ year (not since 1900)
        //data->dateTime.setMonth( _tm->tm_mon + 1 ); // DateTime numbers months from 1 to 12, not 0 to 11
        //data->dateTime.setDayOfMonth( _tm->tm_mday + 1 ); // DateTime numbers days from 1 to 31, not 0 to 30
        //data->dateTime.setHour( _tm->tm_hour );
        //data->dateTime.setMinute( _tm->tm_min );
        //data->dateTime.setSecond( _tm->tm_sec );

        // Sun color and altitude little bit dummy 

         
         osg::Vec4 lightpos = sls->getPosition();
         osg::Vec3 lightDir = sls->getDirection();

         auto _sunAltitude = data->data[0].alt;
         auto _sunColor    = sls->getDiffuse(); 
        // try to calculate some diffuse term (base luminance level for diffuse term is [0, 0.9])
        static const float g_fDiffuseMax = 0.9f;

        osg::Vec3 _globalDiffuse = osg::Vec3(_sunColor.x(),_sunColor.y(),_sunColor.z());// _sunColor;
        // let's additionally fade diffuse when sun is absent
        const float
            fBelowHorizonFactorR = pow(cg::clamp(-2.0f, 8.0f, 0.f, 1.f)(_sunAltitude), 1.25f),
            fBelowHorizonFactorG = powf(fBelowHorizonFactorR, 1.2f),
            fBelowHorizonFactorB = powf(fBelowHorizonFactorR, 1.5f);
        _globalDiffuse = osg::componentMultiply(_globalDiffuse,osg::Vec3(fBelowHorizonFactorR,fBelowHorizonFactorG,fBelowHorizonFactorB));
        //_globalDiffuse.g *= fBelowHorizonFactorG;
        //_globalDiffuse.b *= fBelowHorizonFactorB;
        _globalDiffuse *= g_fDiffuseMax;
///////////////////////////////////////////////////////////////////////////////////////////////
//       В порядке бреда
///////////////////////////////////////////////////////////////////////////////////////////////
        // ambient-diffuse when fog or overcast - increase ambient, decrease diffuse
        const float fDiffuseOvercast = std::max(_fogLayer->getFogDensity()*_fogLayer->getFogDensity(), _skyClouds->getOvercastCoef());
        auto cFogDifCut = sls->getDiffuse()* (0.55f * fDiffuseOvercast);//osg::Vec4f(_globalDiffuse * (0.55f * fDiffuseOvercast),1.0f);
        // decrease diffuse with fog
        auto cFogDif = sls->getDiffuse() - cFogDifCut;
        // increase ambient with fog
        auto cFogAmb = sls->getAmbient() +  cFogDifCut * 0.5f;
        // dim specular
        const float fSpecularOvercastCoef = pow(1.f - fDiffuseOvercast, 0.5f);
        auto cFogSpec = sls->getSpecular() * fSpecularOvercastCoef;

        // recalc illumination based on new foggy values
        const float fIllumDiffuseFactor = 1.f - _skyClouds->getOvercastCoef();
        auto illumination = cg::luminance_crt(cFogAmb + cFogDif * fIllumDiffuseFactor);
        // FIXME Надо передавать в программы скоррестированые значения освещения 

        // when ambient is low - get it's color directly (to make more realistic fog at dusk/dawn)
        // also when overcast - modulate color with ambient
        const float fFogDesatFactor = cg::bound(cFogAmb.x() * 1.5f, 0.f, 1.f);
        //auto fog_color = cg::lerp01(fFogDesatFactor,/*cFogAmb*/cFogDif/*osg::Vec4f(_globalDiffuse,1.0)*/, osg::Vec4f(illumination,illumination,illumination,1.0) );
        auto fog_color = cg::lerp01(/*cFogAmb*/cFogDif/*osg::Vec4f(_globalDiffuse,1.0)*/, osg::Vec4f(illumination,illumination,illumination,1.0),fFogDesatFactor );

        // save fog
         _fogLayer->setFogParams( osg::Vec3(fog_color.x(),fog_color.y(),fog_color.z()), _fogLayer->getFogDensity());
        //data_.fog_exp_coef = _fogLayer->getFogExp2Coef();

        const float fCloudLum = cg::max(0.06f, illumination);
        _skyClouds->setCloudsColors(osg::Vec3f(fCloudLum,fCloudLum,fCloudLum), osg::Vec3f(fCloudLum,fCloudLum,fCloudLum));
       
        _skyClouds->setRotationSiderealTime(-float(fmod(data->localSiderealTime / 24.0, 1.0)) * 360.0f);

        // creators::GetTextureHolder().GetEnvTexture()->apply();
    }
    
    osg::ref_ptr<osgGA::GUIEventHandler> GetHandler() {return _handler.release();};

private:
    class handler : public osgGA::GUIEventHandler
    {

    public:  
        handler(avSky::CloudsLayer* skyClouds) 
              : _skyClouds  (skyClouds)
              , _currCloud  (avSky::cirrus)
              {}

        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
        {
            if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
            {
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Insert)
                {
                    if(_skyClouds)
                    {
                        int cc = _currCloud;cc++;
                        _currCloud = static_cast<avSky::cloud_type>(cc);
                        if(_currCloud >= avSky::clouds_types_num)
                            _currCloud = avSky::none;
                
                        _skyClouds->setCloudsTexture(_currCloud);
                    }
                    return true;
                }
                else
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Up)
                {
                    if(_skyClouds) _skyClouds->setCloudsDensity(cg::bound(_skyClouds->getCloudsDensity() +.1f,.0f,1.0f));

                    return true;
                }
                else
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Down)
                {
                    if(_skyClouds) _skyClouds->setCloudsDensity(cg::bound(_skyClouds->getCloudsDensity() -.1f,.0f,1.0f));

                    return true;
                }
            }
            return false;
        } 

    private:
        osg::ref_ptr<avSky::CloudsLayer>                  _skyClouds;
        avSky::cloud_type                                 _currCloud;
    };

private:
    osg::ref_ptr<osgEphemeris::EphemerisModel> _ephem;
    osg::ref_ptr<avSky::FogLayer>              _fogLayer;
    osg::ref_ptr<avSky::CloudsLayer>           _skyClouds;
    osg::ref_ptr<handler>                      _handler;
    
};

// This handler lets you control the passage of time using keys.
// TODO: add keys to make the time increment by a given increment each frame
// with control over the increment.
class TimeChangeHandler : public osgGA::GUIEventHandler
{
public:
    TimeChangeHandler(osgEphemeris::EphemerisModel *ephem) : m_ephem(ephem) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add)
            {
                // Increment time
                // Hopefully the DateTime will wrap around correctly if we get 
                // to invalid dates / times...
                
                // Модификация Дельты не имеет общих данных  
                //osgEphemeris::EphemerisData* data = m_ephem->getEphemerisData();
                //if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                //    data->dateTime.setHour( data->dateTime.getHour() + 1 );
                //else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                //    data->dateTime.setDayOfMonth( data->dateTime.getDayOfMonth() + 1 );
                //else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                //    data->dateTime.setMonth( data->dateTime.getMonth() + 1 );
                //else                                                                    // Increment by one minute
                //    data->dateTime.setMinute( data->dateTime.getMinute() + 1 );

                osgEphemeris::DateTime dt = m_ephem->getDateTime();
                
                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                    dt.setHour( dt.getHour() + 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                    dt.setDayOfMonth( dt.getDayOfMonth() + 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                    dt.setMonth( dt.getMonth() + 1 );
                else                                                                    // Increment by one minute
                    dt.setMinute( dt.getMinute() + 1 );

                m_ephem->setDateTime(dt);
                //std::cout << "**** Sun light ****" << std::endl;
                //auto sls = m_ephem->getSunLightSource()->getLight();

                //std::cout << "Ambient: " << sls->getAmbient() << std::endl;
                //std::cout << "Diffuse: " << sls->getDiffuse() << std::endl;
                //std::cout << "Specular: " << sls->getSpecular() << std::endl;

                //std::cout << "********************" << std::endl;
                //
                //osg::Vec4 a   = sls->getAmbient();
                //osg::Vec4 d   = sls->getDiffuse();
                //a[3] = 0;//(d[0] + d[1] + d[2])/3.0;
                //sls->setAmbient(a);

                return true;
            }

            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract)
            {
                // Decrement time
                // Hopefully the DateTime will wrap around correctly if we get 
                // to invalid dates / times...
                //osgEphemeris::EphemerisData* data = m_ephem->getEphemerisData();
                //if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                //    data->dateTime.setHour( data->dateTime.getHour() - 1 );
                //else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                //    data->dateTime.setDayOfMonth( data->dateTime.getDayOfMonth() - 1 );
                //else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                //    data->dateTime.setMonth( data->dateTime.getMonth() - 1 );
                //else                                                                    // Decrement by one minute
                //    data->dateTime.setMinute( data->dateTime.getMinute() - 1 );
                osgEphemeris::DateTime dt = m_ephem->getDateTime();
                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                    dt.setHour( dt.getHour() - 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                    dt.setDayOfMonth( dt.getDayOfMonth() - 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                    dt.setMonth( dt.getMonth() - 1 );
                else                                                                    // Decrement by one minute
                    dt.setMinute( dt.getMinute() - 1 );

                m_ephem->setDateTime(dt);
                
                return true;
            }
            else
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_I)
            {

                osgEphemeris::EphemerisData* data = m_ephem->getEphemerisData();
                
                data->turbidity += 1 ;

                return true;
            }
            else
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_O)
            {
                osgEphemeris::EphemerisData* data = m_ephem->getEphemerisData();
                                                                                     // Increment by one minute
                data->turbidity -= 1 ;

                return true;
            }

        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Increment time by one minute");
        usage.addKeyboardMouseBinding("Shift Keypad +", "Increment time by one hour"  );
        usage.addKeyboardMouseBinding("Alt Keypad +",   "Increment time by one day"   );
        usage.addKeyboardMouseBinding("Ctrl Keypad +",  "Increment time by one month" );
        usage.addKeyboardMouseBinding("Keypad -",       "Decrement time by one minute");
        usage.addKeyboardMouseBinding("Shift Keypad -", "Decrement time by one hour"  );
        usage.addKeyboardMouseBinding("Alt Keypad -",   "Decrement time by one day"   );
        usage.addKeyboardMouseBinding("Ctrl Keypad -",  "Decrement time by one month" );
    }

    osg::ref_ptr<osgEphemeris::EphemerisModel> m_ephem;
};


#if 1
osg::Node*
    preRender( osg::Node* node)
{
    const int tex_width = 512, tex_height = 512;

    osg::ref_ptr<osg::Texture2D> textureFBO = new osg::Texture2D;
    textureFBO->setTextureSize( tex_width, tex_height );
    // textureFBO->setInternalFormat( GL_DEPTH_COMPONENT24 );  // GL_RGB   // FIXME GL_RGBA8 - интересный эффект проге плохеет
    // Чет совсем плохо с в форматами   GL_DEPTH_COMPONENT24 тоже изжогу вызывает
    
    textureFBO->setSourceFormat(GL_DEPTH_COMPONENT); 
    textureFBO->setInternalFormat(GL_DEPTH_COMPONENT32); 
    textureFBO->setSourceType(GL_UNSIGNED_INT); 
    
    //textureFBO->setInternalFormat(GL_DEPTH_COMPONENT); 

    textureFBO->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR/*NEAREST*/ );
    textureFBO->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR/*NEAREST*/ );
    textureFBO->setBorderColor(osg::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
    textureFBO->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
    textureFBO->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
    // NB GL_COMPARE_REF_TO_TEXTURE и GL_COMPARE_R_TO_TEXTURE are equal
    textureFBO->setShadowComparison(true);            // Sets GL_TEXTURE_COMPARE_MODE_ARB to GL_COMPARE_R_TO_TEXTURE_ARB
    textureFBO->setShadowCompareFunc(osg::Texture::LESS);
    


    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setViewport( 0, 0, tex_width, tex_height );
    //camera->setClearColor( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
    //camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
    
    //camera->setCullingMode(osg::CullSettings::NO_CULLING);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    //camera->setClearDepth(0.0f);
    //camera->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::GEQUAL, 1.0, 0.0, true), osg::StateAttribute::OVERRIDE);

    camera->setRenderOrder( osg::Camera::PRE_RENDER );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->attach( osg::Camera::DEPTH_BUFFER/*COLOR_BUFFER*/, textureFBO.get() );
    // camera->setProjectionMatrixAsFrustum(-1.f, +1.f, -1.f, +1.f, 1.f, 10.f);
    // camera->setViewMatrixAsLookAt(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0,1,0), osg::Z_AXIS);
    
    camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    camera->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
    cf->setMode(osg::CullFace::FRONT);
    camera->getStateSet()->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);      
    
    camera->addChild( node );


    return( camera.release() );
} 
#else
osg::Node*
    preRender( osg::Node* node)
{
    const int tex_width = 512, tex_height = 512;

    osg::ref_ptr<osg::Texture2D> textureFBO = new osg::Texture2D;
    textureFBO->setTextureSize( tex_width, tex_height );
    textureFBO->setInternalFormat(GL_DEPTH_COMPONENT);
    textureFBO->setShadowComparison(true);
    textureFBO->setShadowTextureMode(osg::Texture2D::LUMINANCE);
    textureFBO->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    textureFBO->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // the shadow comparison should fail if object is outside the texture
    textureFBO->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    textureFBO->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    textureFBO->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));



    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setViewport( 0, 0, tex_width, tex_height );
    //camera->setClearColor( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
    //camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    //camera->setCullingMode(osg::CullSettings::NO_CULLING);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    //camera->setClearDepth(0.0f);
    //camera->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::GEQUAL, 1.0, 0.0, true), osg::StateAttribute::OVERRIDE);

    camera->setRenderOrder( osg::Camera::PRE_RENDER );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->attach( osg::Camera::DEPTH_BUFFER/*COLOR_BUFFER*/, textureFBO.get() );
    // camera->setProjectionMatrixAsFrustum(-1.f, +1.f, -1.f, +1.f, 1.f, 10.f);
    // camera->setViewMatrixAsLookAt(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0,1,0), osg::Z_AXIS);

    camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    
    osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
    cf->setMode(osg::CullFace::FRONT);
    camera->getStateSet()->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);      
    camera->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    camera->addChild( node );


    return( camera.release() );
} 

#endif

osg::Node*
    decalRender( osgViewer::Viewer& viewer)
{
    osg::Camera* rootCamera( viewer.getCamera() );

    // Create the texture; we'll use this as our color buffer.
    // Note it has no image data; not required.
    osg::Texture2D* tex = creators::getTextureHolder().getDecalTexture().get();// new osg::Texture2D;


    // Attach the texture to the camera. Tell it to use multisampling.
    // Internally, OSG allocates a multisampled renderbuffer, renders to it,
    // and at the end of the frame performs a BlitFramebuffer into our texture.
    //rootCamera->attach( osg::Camera::COLOR_BUFFER0, tex, 0, 0, false, 8, 8 );
    //rootCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
#if( OSGWORKS_OSG_VERSION >= 20906 )
    rootCamera->setImplicitBufferAttachmentMask(
        osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT|osg::Camera::IMPLICIT_DEPTH_BUFFER_ATTACHMENT,
        osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT );
#endif


    // Configure postRenderCamera to draw fullscreen textured quad
    osg::ref_ptr< osg::Camera > postRenderCamera( new osg::Camera );

    postRenderCamera->setClearMask( 0 );
    postRenderCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
    postRenderCamera->attach( osg::Camera::COLOR_BUFFER/*COLOR_BUFFER0*/, tex);// , 0, 0, false, 8, 8 );

    postRenderCamera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    postRenderCamera->setRenderOrder( osg::Camera::PRE_RENDER );
    postRenderCamera->setViewMatrix( osg::Matrixd::identity() );
    postRenderCamera->setProjectionMatrix( osg::Matrixd::identity() );
   
    //osg::Geode* geode( new osg::Geode );
    //geode->addDrawable( osgwTools::makePlane(
    //    osg::Vec3( -1,-1,0 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,2,0 ) ) );
    //geode->getOrCreateStateSet()->setTextureAttributeAndModes(
    //    0, tex, osg::StateAttribute::ON );
    //geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    //geode->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    //postRenderCamera->addChild( geode );

    return( postRenderCamera.release() );
}



int main_scene( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

#if 0
    // create the window to draw to.
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = 1920;
    traits->height = 1080;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (!gw)
    {
        osg::notify(osg::NOTICE)<<"Error: unable to create graphics window."<<std::endl;
        return 1;
    }
#endif

#ifdef TEST_CAMERA
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    camera->setViewMatrixAsLookAt(
        osg::Vec3(0.0f,-5.0f,5.0f), osg::Vec3(),
        osg::Vec3(0.0f,1.0f,1.0f)
        );
    
#endif
    
    spark::init();

	osg::DisplaySettings::instance()->setNumMultiSamples( 8 );

    osgViewer::Viewer viewer(arguments);
    arguments.reportRemainingOptionsAsUnrecognized();
    //viewer.setUpViewOnSingleScreen(1);
    viewer.apply(new osgViewer::SingleScreen(1));
	
        // Set the clear color to black
    viewer.getCamera()->setClearColor(osg::Vec4(0,0,0,1));

#if 0
    viewer.getCamera()->setGraphicsContext(gc.get());
    viewer.getCamera()->setViewport(0,0,1920,1080);
#endif

    // tilt the scene so the default eye position is looking down on the model.
    osg::ref_ptr<osg::MatrixTransform> rootnode = new osg::MatrixTransform;
    //rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),1.0f,0.0f,0.0f));

    // Use a default camera manipulator
    osgGA::FirstPersonManipulator* manip = new osgGA::FirstPersonManipulator;
    
    manip->setAcceleration(0);
    manip->setMaxVelocity(1);
    manip->setWheelMovement(0.001,true);
    viewer.setCameraManipulator(manip);
    // Initially, place the TrackballManipulator so it's in the center of the scene
    manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
    //manip->home(0);

    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(rootnode->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);

    // any option left unread are converted into errors to write out later.


    // report any errors if they have occurred when parsing the program arguments.
    //if (arguments.errors())
    //{
    //    arguments.writeErrorMessages(std::cout);
    //    return 1;
    //}

    std::string animationName("Default");
    
    const osg::Quat quat0(0,          osg::X_AXIS,                      
                          0,          osg::Y_AXIS,
                          0,          osg::Z_AXIS ); 

    // auto model = osgDB::readNodeFile("an_124.dae");  /*a_319.3ds*/
    bool overlay = false;
    osgSim::OverlayNode::OverlayTechnique technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY;//  osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    //while (arguments.read("--object")) { technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY; overlay=true; }
    //while (arguments.read("--ortho") || arguments.read("--orthographic")) { technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY; overlay=true; }
    //while (arguments.read("--persp") || arguments.read("--perspective")) { technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY; overlay=true; }
    
    osg::ref_ptr<osg::LightSource> ls;

    // load the nodes from the commandline arguments.
    auto model_parts  = creators::createModel(ls, overlay, technique);
    
    osg::ref_ptr<osg::MatrixTransform> turn_node = new osg::MatrixTransform;
    //turn_node->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),1.0f,0.0f,0.0f));
    turn_node->addChild(model_parts[0]);
    osg::Node* model = turn_node;//model_parts[0];

    if(model == nullptr)
    {
         osg::notify(osg::WARN) << "Can't load " <<  "an_124.dae";
        return 1;
    }
    else
    {
        auto ct =  findFirstNode(model,"camera_tower");

        if(ct) 
            manip->setHomePosition(ct->getBound().center(), osg::Vec3(0,1,0), osg::Z_AXIS);



        osg::BoundingSphere loaded_bs = model->getBound();

        // create a bounding box, which we'll use to size the room.
        osg::BoundingBox bb;
        bb.expandBy(loaded_bs);

 #ifdef TEST_SKYBOX
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(
            new osg::Sphere(osg::Vec3(), model->getBound().radius())) );
        geode->setCullingActive( false );


        osg::ref_ptr<SkyBox> skybox = new SkyBox;
        skybox->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::TexGen );
        skybox->setEnvironmentMap( 0,
            osgDB::readImageFile("Cubemap_snow/posx.jpg"), osgDB::readImageFile("Cubemap_snow/negx.jpg"),
            osgDB::readImageFile("Cubemap_snow/posy.jpg"), osgDB::readImageFile("Cubemap_snow/negy.jpg"),
            osgDB::readImageFile("Cubemap_snow/posz.jpg"), osgDB::readImageFile("Cubemap_snow/negz.jpg") );
        skybox->addChild( geode.get() );
#endif

#ifdef TEST_SHADOWS        
        // Light.
        //osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
        //source->getLight()->setPosition(osg::Vec4(0, 0, 0, 0));
        //source->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
        //source->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));

        int shadowsize = 1024*2/**16*/;//1024;
        osg::ref_ptr<osgShadow::SoftShadowMap> sm = new osgShadow::SoftShadowMap;
        sm->setTextureSize(osg::Vec2s(shadowsize, shadowsize));
        sm->setTextureUnit(1);
        sm->setJitteringScale(16);
        // Scene
        osg::ref_ptr<osgShadow::ShadowedScene> root = new osgShadow::ShadowedScene;
        root->setShadowTechnique(sm.get());
        root->addChild(model);

        //model->asGroup()->addChild(source);
        rootnode->addChild(root); 
        
#else
        rootnode->addChild(model);
#endif
      


#ifdef TEST_SKYBOX
        rootnode->addChild( skybox.get() );
#endif

#ifdef TEST_OUTLINE
        {
            // create outline effect
            osg::ref_ptr<osgFX::Outline> outline = new osgFX::Outline;
            model_parts[1]->asGroup()->addChild(outline.get());

            outline->setWidth(4);
            outline->setColor(osg::Vec4(1,1,0,1));
            outline->addChild(model_parts[3]);
        }
#endif

         osg::ref_ptr<osg::LightSource> sun_light;

#ifdef  TEST_EPHEMERIS
        osg::BoundingSphere bs = model->getBound();
        osg::ref_ptr<osgEphemeris::EphemerisModel> ephemerisModel = new osgEphemeris::EphemerisModel;

		ephemerisModel->setSkyDomeMirrorSouthernHemisphere(false);
		// ephemerisModel->setSkyDomeUseSouthernHemisphere(true);

        // Optionally, Set the AutoDate and Time to false so we can control it with the GUI
        ephemerisModel->setAutoDateTime( false );


        // Set some acceptable defaults.
        double latitude = 43.4444;                                  // Adler, RF
        double longitude = 39.9469;
        osgEphemeris::DateTime dateTime( 2010, 4, 1, 8, 0, 0 );     // 8 AM
        double radius = 10000;                                      // Default radius in case no files were loaded above
        osg::BoundingSphere bs_root = rootnode->getBound();
        if (bs_root.valid())                                             // If the bs is not valid then the radius is -1
            radius = bs_root.radius()*2;                                 // which would result in an inside-out skydome...

        ephemerisModel->setLatitudeLongitude( latitude, longitude );
        ephemerisModel->setDateTime( dateTime );
        ephemerisModel->setSkyDomeRadius( radius );
         // Optionally, uncomment this if you want to move the Skydome, Moon, Planets and StarField with the mouse
		ephemerisModel->setMoveWithEyePoint(false);
        // sun_light = ephemerisModel->getSunLightSource(); 
        // ephemerisModel->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        //osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
        //ephemerisModel->getOrCreateStateSet()->setAttribute(pDepth,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        rootnode->addChild( ephemerisModel.get() );

        time_t seconds = time(0L);
        struct tm *_tm = localtime(&seconds);
        osgEphemeris::DateTime dt;
        dt.setYear( _tm->tm_year + 1900 ); // DateTime uses _actual_ year (not since 1900)
        dt.setMonth( _tm->tm_mon + 1 ); // DateTime numbers months from 1 to 12, not 0 to 11
        dt.setDayOfMonth( _tm->tm_mday + 1 ); // DateTime numbers days from 1 to 31, not 0 to 30
        dt.setHour( _tm->tm_hour );
        dt.setMinute( _tm->tm_min );
        dt.setSecond( _tm->tm_sec );
        ephemerisModel->setDateTime( dt );

#endif  // TEST_EPHEMERIS


#ifdef  TEST_OSG_FOG 
        // Да не плохо конечно только материалы должны быть без шейдеров
        osg::ref_ptr<osg::Fog> fog = new osg::Fog;
        fog->setMode( osg::Fog::LINEAR );
        fog->setStart( 500.0f );
        fog->setEnd( 2500.0f );
        fog->setColor( osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f) );

        rootnode->getOrCreateStateSet()->setAttributeAndModes( fog.get() );
#endif

#ifdef TEST_SV_FOG   
         osg::ref_ptr<avSky::FogLayer> skyFogLayer = new avSky::FogLayer(/*ephemerisModel*/model->asGroup());
         ephemerisModel->asGroup()->addChild(skyFogLayer.get());
         auto fogColor =osg::Vec3f(1.0,1.0,1.0);
         skyFogLayer->setFogParams(fogColor,0.1);    // (начинаем с 0.1 до максимум 1.0)
         float coeff = skyFogLayer->getFogExp2Coef();
         viewer.addEventHandler( new avSky::FogHandler(
             [&](osg::Vec4f v){
                 skyFogLayer->setFogParams(osg::Vec3f(v.x(),v.y(),v.z()),v.w());
                 float coeff = skyFogLayer->getFogExp2Coef();
                 char str[255];
                 sprintf(str,"setFogParams coeff = %f",coeff);
                 osg::notify(osg::INFO) << str;
             }
             , fogColor ));
#else
        osg::ref_ptr<avSky::FogLayer> skyFogLayer;
#endif

#ifdef TEST_SV_CLOUD
        osg::ref_ptr<avSky::CloudsLayer> cloudsLayer = new avSky::CloudsLayer(/*ephemerisModel*/model->asGroup());
        ephemerisModel->asGroup()->addChild(cloudsLayer.get());
        cloudsLayer->setCloudsColors( osg::Vec3f(1.0,1.0,1.0), osg::Vec3f(1.0,1.0,1.0) );
#else 
        osg::ref_ptr<CloudsLayer> cloudsLayer;
#endif

             

#ifdef  TEST_EPHEMERIS  
             // Optionally, use a callback to update the Ephemeris Data
             osg::ref_ptr<EphemerisDataUpdateCallback> eCallback = new EphemerisDataUpdateCallback(ephemerisModel.get(),skyFogLayer.get(),cloudsLayer.get());
             ephemerisModel->setEphemerisUpdateCallback( eCallback );

             osg::ref_ptr<osg::Group> fbo_node = new osg::Group;
             fbo_node->addChild(ephemerisModel.get());
             rootnode->addChild(avEnv::createPrerender(fbo_node,osg::NodePath(),0,osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f),osg::Camera::FRAME_BUFFER_OBJECT));
#endif

#ifdef  TEST_EPHEMERIS
             ephemerisModel->setSunLightSource(ls); 
#endif 

            auto dr = decalRender(viewer);
            
            
            osg::ref_ptr<osg::Geode> tn = new osg::Geode;
            tn->addDrawable( new TeapotDrawable(1.0f) );
            dr->asTransform()->addChild(tn);
            rootnode->addChild(tn);
            rootnode->addChild(dr);

#ifdef TEST_SHADOWS_2  // TEST_FBO

             //osg::ref_ptr<osg::Group> fbo_shadow_node = new osg::Group;
             //fbo_shadow_node->addChild(model);
             //rootnode->addChild(preRender( fbo_shadow_node));
             
             osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
             source->getLight()->setPosition(osg::Vec4(0, /*0.001*/0, 20, 0));    // osg::Vec4(0, 20 , 0, 0) - вид с боку
             source->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
             source->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));

             ShadowMap* sm = creators::GetShadowMap();    
             sm->setLightGetter([&]()->osg::Light* {return source->getLight() /*ephemerisModel->getSunLightSource()->getLight()*/;});
             sm->setScene(model);
             rootnode->addChild(sm);

             //osg::ref_ptr<osg::Group> g = new osg::Group;
             //osg::ref_ptr<Prerender>  p = new Prerender(1024,1024);
             //p->asGroup()->addChild(fbo_node);
             //g->addChild(p);
             //rootnode->addChild(g);
             
#endif

#ifdef TEST_SHADOWS
        if (sun_light.valid())
            sm->setLight(sun_light.get());
#endif

//#ifdef  TEST_EPHEMERIS 
//        if(ssm.valid())
//        {
//            
//            // ssm->setLight(sun_light->getLight()/*.get()*/);
//            eCallback->extCallback( [&ephemerisModel,&ssm,&rootnode]()->void 
//            {   
//                
//                auto l = ephemerisModel->getSunLightSource()->getLight();
//                // rootnode->addChild(ephemerisModel->getSunLightSource());
//                //ssm->setLight(l);
//            });
//        }
//#endif

        

        //AddLight(rootnode);

        // run optimization over the scene graph
        //osgUtil::Optimizer optimzer;
        //optimzer.optimize(rootnode);
        
        // osgDB::writeNodeFile(*model,"test_osg_struct.osgt");

        spark::spark_pair_t sp =   spark::create(spark::FIRE);
        spark::spark_pair_t sp2 =  spark::create(spark::EXPLOSION);

        osg::MatrixTransform* posed = new osg::MatrixTransform(osg::Matrix::translate(osg::Vec3(400.0,400.0,50.0)));
        posed->addChild(sp.first);
        posed->addChild(sp2.first);

        rootnode->addChild(posed);

        // set the scene to render
        viewer.setSceneData(rootnode);

        // must clear stencil buffer...
        unsigned int clearMask = viewer.getCamera()->getClearMask();
        viewer.getCamera()->setClearMask(clearMask | GL_STENCIL_BUFFER_BIT);
        viewer.getCamera()->setClearStencil(0);

#ifdef ENABLE_CUSTOM_ANIMATION
        std::list<std::string>  l; 
        auto root_group = model->asGroup();
        if(root_group)
        for(unsigned int i=0; i< root_group->getNumChildren(); ++i)
        {
            std::cout << "Child nodes: " << root_group->getChild(i)->getName() << std::endl;
            l.push_back( root_group->getChild(i)->getName());
            if(l.back() == "Shassis_LO")
            {
                root_group->getChild(i)->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); 
                root_group->getChild(i)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                root_group->getChild(i)->getStateSet()->setRenderBinDetails(1, "DepthSortedBin"); 
                auto shassy = root_group->getChild(i)->asGroup();
                for(unsigned j=0; j< shassy->getNumChildren(); ++j)
                {
                     shassy->getChild(j)->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); 
                     shassy->getChild(j)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                     shassy->getChild(j)->getStateSet()->setRenderBinDetails(1, "DepthSortedBin"); 

                     std::string s_name = shassy->getChild(j)->getName();
                     if (s_name=="lg_left" || s_name=="lg_right" || s_name=="lg_forward" || s_name=="lp18" || s_name=="lp21")
                     {
                         int sign = (s_name=="lg_left" || s_name=="lp21")?1:(s_name=="lg_right" || s_name=="lp18")?-1:1;
                         auto mt_ = shassy->getChild(j)->asTransform()->asMatrixTransform();
                         //auto pos = shassy->getChild(j)->asTransform()->asPositionAttitudeTransform();->getPosition()
                         {
                             osg::Quat quat;
                             if(s_name!="lg_forward")
                             {
                                     quat = osg::Quat (0,   osg::X_AXIS,
                                                       0,   osg::Y_AXIS,
                                 sign*(0.8 * osg::PI_2 ),   osg::Z_AXIS );
                             }
                             else
                             {
                                      quat = osg::Quat (osg::PI_2*1.5,   osg::X_AXIS,
                                                                    0,   osg::Y_AXIS,
                                                                    0,   osg::Z_AXIS );
                                
                             }
                             
                             auto bound = shassy->getChild(j)->getBound();
                             // set up the animation path
                             osg::AnimationPath* animationPath = new osg::AnimationPath;
                             animationPath->insert(0.0,osg::AnimationPath::ControlPoint(bound.center()+ osg::Vec3d(0,bound.radius()/2,0)/* *computeTargetToWorldMatrix(shassy->getChild(j))*/,quat));
                             animationPath->insert(1.0,osg::AnimationPath::ControlPoint(bound.center()+ osg::Vec3d(0,bound.radius()/2,0)/* *computeTargetToWorldMatrix(shassy->getChild(j))*/,quat0));
                             animationPath->insert(0.0,osg::AnimationPath::ControlPoint(bound.center() + osg::Vec3d(0,bound.radius()/2,0)/* *computeTargetToWorldMatrix(shassy->getChild(j))*/,quat));

                             //animationPath->insert(1.0,osg::AnimationPath::ControlPoint(osg::Vec3d(1.,1.,1.)*mt_->getMatrix(),quat0));
                             //animationPath->insert(0.0,osg::AnimationPath::ControlPoint(osg::Vec3d(1.,1.,1.)*mt_->getMatrix(),quat));
                         
                             animationPath->setLoopMode(osg::AnimationPath::SWING);

                             mt_->setUpdateCallback(new osg::AnimationPathCallback(animationPath));
                         }


                         auto mat_ =  mt_->getMatrix();

                         //int sign = (s_name=="lp2")?1:(s_name=="lp3")?-1:0; 
                         //osg::Matrix zRot;
                         //zRot.makeRotate(sign*osg::PI_4, 0.0,0.0,1.0);
                                          
                         //mt_->setMatrix( zRot*mat_ );
                     

                         auto shassy_l = shassy->getChild(j)->asGroup();
                         for(unsigned k=0; k< shassy_l->getNumChildren(); ++k)
                         {
                             shassy_l->getChild(k)->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); 
                             shassy_l->getChild(k)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                             shassy_l->getChild(k)->getStateSet()->setRenderBinDetails(1, "DepthSortedBin"); 
                         
                         }
                     }
                }


            }

        }
#endif

        osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
        viewer.addEventHandler(statesetManipulator.get());

        auto node =  findFirstNode(model,"airplane");
        if(node)
            viewer.addEventHandler(new AnimationHandler(/*node*/model_parts[1],animationName
                   ,[&](){effects::insertParticle(model->asGroup(),/*node*/model_parts[2]/*->asGroup()*/,osg::Vec3(00.f,00.f,00.f),0.f);}
                   //,[&](){ 
                   //   
                   //    spark::spark_pair_t sp3 =  spark::create(spark::SMOKE,model->asGroup()->asTransform());
                   //    model->asGroup()->asTransform()->addChild(sp3.first);
                   //    
                   // }
                   ,[&](bool off){model_parts[2]->setUpdateCallback(off?nullptr:new circleAimlessly());}
                   ,[&](bool low){model_parts[4]->setNodeMask(low?0:0xffffffff);model_parts[5]->setNodeMask(low?0xffffffff:0);}
            ));
#ifdef TEST_CAMERA        
        camera->addChild( model_parts[1] );
#endif

#ifdef  TEST_EPHEMERIS
        viewer.addEventHandler( new TimeChangeHandler( ephemerisModel.get() ) );
        viewer.addEventHandler( eCallback->GetHandler() );
#endif

        viewer.addEventHandler( bi::getUpdater().get() );

        viewer.addEventHandler(sp.second);

#if 0
		//Experiment with setting the LightModel to very dark to get better sun lighting effects
		{
			osg::ref_ptr<osg::StateSet> sset = rootnode->getOrCreateStateSet();
			osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel;
			lightModel->setAmbientIntensity( osg::Vec4( 0.0025, 0.0025,0.0025, 1.0 ));
			sset->setAttributeAndModes( lightModel.get() );
		}
#endif

#if 0
		// create the light    
		osg::LightSource* lightSource = new osg::LightSource;
		rootnode->addChild(lightSource);

		osg::Light* light = lightSource->getLight();
		light->setLightNum(2);
		light->setPosition(osg::Vec4(0.0f,0.0f,1.0f,0.0f)); // directional light from above
		light->setAmbient(osg::Vec4(0.8f,0.8f,0.8f,1.0f));
		light->setDiffuse(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
		light->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));

#ifdef  TEST_EPHEMERIS 
        eCallback->setLightSource(lightSource);
#endif
#endif

#ifdef TEST_PRECIP   // здесь у нас будет снег 
	     osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;		
         /*model->asGroup()->*/rootnode->addChild(precipitationEffect.get());
		 precipitationEffect->snow(0.3);
         precipitationEffect->setWind(osg::Vec3(0.2f,0.2f,0.2f));
#endif
		// Useless
		//rootnode->getOrCreateStateSet()->setMode( GL_LINE_SMOOTH, osg::StateAttribute::ON );
		//rootnode->getOrCreateStateSet()->setMode( GL_POLYGON_SMOOTH, osg::StateAttribute::ON );

#if TEST_NODE_TRACKER  // Странный эффект для модели
		osg::ref_ptr<osgGA::NodeTrackerManipulator> manip
			= new osgGA::NodeTrackerManipulator;
		 manip->setTrackNode(model_parts[1]/*.get()*/);
		 manip->setTrackerMode(osgGA::NodeTrackerManipulator::NODE_CENTER);
		 viewer.setCameraManipulator(manip.get());

#endif

#ifdef  PRINT_STRUCTURE
        InfoVisitor infoVisitor;
        model->accept( infoVisitor );
#endif

#if 0
        const GLubyte *renderer = glGetString( GL_RENDERER );
        const GLubyte *vendor = glGetString( GL_VENDOR );
        const GLubyte *version = glGetString( GL_VERSION );
        const GLubyte *glslVersion =
            glGetString( GL_SHADING_LANGUAGE_VERSION );
        GLint major, minor;
        //glGetIntegerv(GL_MAJOR_VERSION, &major);
        //glGetIntegerv(GL_MINOR_VERSION, &minor);
        printf("GL Vendor : %s\n", vendor);
        printf("GL Renderer : %s\n", renderer);
        printf("GL Version (string) : %s\n", version);
        printf("GL Version (integer) : %d.%d\n", major, minor);
        printf("GLSL Version : %s\n", glslVersion);	
#endif

        return viewer.run();
    }

    return 1;
}

AUTO_REG(main_scene)
