#include "stdafx.h"
#include "av/precompiled.h"

#include "utils/high_res_timer.h"
#include "utils/pickhandler.h"
#include "utils/animutils.h"

#if !defined(VISUAL_EXPORTS)
#include "phys/RigidUpdater.h"
#endif

#include <osg/GLObjects>

#include "av/avAnimation/AnimTest.h"

#include "av/avCore/Logo.h"
#include "av/avCore/PreRender.h"

#include "trajectory_drawer.h"

//Degree precision versus length
//
//          decimal                                                                               N/S or E/W    E/W at    E/W at       E/W at
//          degrees           DMS                 qualitative scale that can be identified        at equator    23N/S      45N/S	     67N/S
//    0	    1.0	        1° 00′ 0″	        country or large region	                            111.32 km	102.47 km	78.71 km	43.496 km
//    1	    0.1	        0° 06′ 0″	        large city or district	                            11.132 km	10.247 km	7.871 km	4.3496 km
//    2 	0.01	    0° 00′ 36″	        town or village	                                    1.1132 km	1.0247 km	787.1 m	    434.96 m
//    3	    0.001	    0° 00′ 3.6″	        neighbourhood, street	                            111.32 m	102.47 m	78.71 m	    43.496 m
//    4 	0.0001	    0° 00′ 0.36″	    individual street, land parcel	                    11.132 m	10.247 m	7.871 m	    4.3496 m
//    5	    0.00001	    0° 00′ 0.036″	    individual trees	                                1.1132 m	1.0247 m	787.1 mm	434.96 mm
//    6	    0.000001	0° 00′ 0.0036″	    individual humans	                                111.32 mm	102.47 mm	78.71 mm	43.496 mm
//    7	    0.0000001	0° 00′ 0.00036″	    practical limit of commercial surveying	            11.132 mm	10.247 mm	7.871 mm	4.3496 mm
//    8	    0.00000001	0° 00′ 0.000036″	specialized surveying (e.g. tectonic plate mapping)	1.1132 mm	1.0247 mm	787.1 µm	434.96 µm
      

#include "av/avShadows/ShadowedScene.h"
#include "av/avShadows/ShadowMap.h"
#include "av/avShadows/ViewDependentShadowMap.h"
#include "av/avShadows/ParallelSplitShadowMap.h"
#include "av/avSky/Sky.h"

#include "av/avSky/Ephemeris.h"

#include "av/avScene/Scene.h"
#include "av/avScene/ScreenTextureManager.h"

#include "av/avLights/Lights.h"
#include "av/avLights/LightManager.h"
#include "av/avLights/NavAid.h"

#include "av/avLights/CulturalLights/PointLightsManager.h"
#include "av/avLights/CulturalLights/PointLightGroup.h"
#include "av/avLights/CulturalLights/NavigationalLight.h"

#include "av/avTerrain/Terrain.h"

#include "av/avWeather/Weather.h" 

#include "av/avCore/Object.h"
#include "av/avCore/Random.h"

#include "application/panels/vis_settings_panel.h"
#include "application/panels/time_panel.h"

#include "application/main_window.h"
#include "application/menu.h"

#include "tests/common/CommonFunctions"
#include "tests/creators.h"

#include "utils/async_load.h"
#include "utils/LoadManager.h"

#include "av/avFx/SmokeFx.h"
#include "av/avFx/SparksFx.h"
#include "av/avFx/LandingDustFx.h"
#include "av/avFx/FoamStreamFx.h"
#include "av/avFx/FrictionDustFx.h"

//
//  ext
//
#include "spark/osgspark.h"

//#define PPU_TEST
#ifdef PPU_TEST
#include "tests/simple.h"
#include <osgPPU/UnitText.h>
#pragma comment(lib, "osgPPU.lib")
#undef TEST_SHADOWS_FROM_OSG
#endif


#define POINT_LIGHT_MANAGER
#define DECAL_RENDERER     
#define TRAJECTORY_DRAWER

#define TEST_WEATHER


#define TEST_ANIMATION

#ifdef TEST_ANIMATION
    namespace tests {
    osg::Group* createAnimatedObject();
    }
#endif


namespace gui 
{ 
    osgGA::GUIEventHandler*  createCEGUI(osg::Group* root, std::function<void()> init_gui_handler);
    void    releaseCEGUI();
}

using namespace avScene;

    // Useless
#if 0
namespace {
    cg::transform_4 get_relative_transform( osg::Node* node, osg::Node* rel=nullptr )
    {
        osg::Matrix tr;
        osg::Node* n = node;
        //osg::Node* root = nullptr;

        while( 0 != n->getNumParents() && n->getName() != "phys_ctrl" && (rel?n != rel:true)  )
        {
            if(n->asTransform())
                if(n->asTransform()->asMatrixTransform())
                {
                    tr = n->asTransform()->asMatrixTransform()->getMatrix() * tr;
                }
                else
                {
                    osg::Matrix matrix;
                    const osg::PositionAttitudeTransform* pat = n->asTransform()->asPositionAttitudeTransform();
                    matrix.setTrans(pat->getPosition());
                    tr =  matrix * tr;
                }

            n = n->getParent(0);
        }

        if (rel == NULL || n == rel  )
            return from_osg_transform(tr);

        osg::Matrix tr_rel;
        n = rel;
        while(0 != n->getNumParents() && n->getName() != "phys_ctrl"/*root->getName()*/)
        {                  
            if(n->asTransform())
                if(n->asTransform()->asMatrixTransform())
                {
                    tr_rel = n->asTransform()->asMatrixTransform()->getMatrix() * tr_rel;
                }
                else
                {
                    osg::Matrix matrix;
                    const osg::PositionAttitudeTransform* pat = n->asTransform()->asPositionAttitudeTransform();
                    matrix.setTrans(pat->getPosition());
                    tr_rel = matrix * tr_rel;
                }

            n = n->getParent(0);
        }

        return from_osg_transform((osg::Matrix::inverse(tr_rel)) * tr);
    }

}
#endif

namespace {


osg::Geode* CreateLight (const osg::Vec4& fcolor,const std::string& name,osg::NodeCallback* callback)
{
    static std::map<osg::Vec4,osg::Geode*> colors;
    if(colors.find ( fcolor ) == colors.end())
    {
        osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
        hints->setDetailRatio(0.1f); 
        hints->setTargetNumFaces(2);
        osg::ref_ptr<osg::ShapeDrawable> shp = new osg::ShapeDrawable(nullptr,hints);
        shp->setShape( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.3f) );
        osg::Geode* light = new osg::Geode;
        light->addDrawable( shp.get() );
        dynamic_cast<osg::ShapeDrawable *>(light->getDrawable(0))->setColor( fcolor );
        light->setUpdateCallback(callback);
        light->setName(name);
        light->setCullingActive(false);
        colors [fcolor] = light;
    }

#if 0  // Перенесено см ниже
    const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED| osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF;
    osg::StateSet* ss = light->getOrCreateStateSet();
    ss->setAttribute(new osg::Program(),value);
    ss->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
    ss->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    // ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    ss->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");
    // ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF |  osg::StateAttribute::PROTECTED);
#endif    

    return  colors [fcolor];
};

inline cg::vector_3f set_direction(const float pitch, const float heading)
{
	return cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));
}

}

class DebugHMIHandler : public osgGA::GUIEventHandler
{

public:  
    DebugHMIHandler(avSky::Sky* sky) 
        : _currCloud  (av::weather_params::cirrus)
        , _intensivity(0.1)
        , _sky(sky)
    {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        auto  const _skyClouds  = avCore::GetEnvironment()->GetWeatherParameters().CloudType;
        const osg::Vec3f _color = osg::Vec3f(1.0,1.0,1.0);   // FIXME Whats color?

        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Insert)
            {
                //if(_skyClouds)
                {
                    int cc = _currCloud;cc++;
                    _currCloud = static_cast<av::weather_params::cloud_type>(cc);
                    if(_currCloud >= av::weather_params::clouds_types_num)
                        _currCloud = av::weather_params::none;
                    
                    avCore::GetEnvironment()->m_WeatherParameters.CloudType = _currCloud;
                }
                return true;
            }
            else
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Up)
                {
                    
                    avCore::GetEnvironment()->m_WeatherParameters.CloudDensity = cg::bound(avCore::GetEnvironment()->GetWeatherParameters().CloudDensity +.1f,.0f,1.0f);
                    return true;
                }
                else
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Down)
                    {
                        avCore::GetEnvironment()->m_WeatherParameters.CloudDensity = cg::bound(avCore::GetEnvironment()->GetWeatherParameters().CloudDensity -.1f,.0f,1.0f);

                        return true;
                    }
                    else
                        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add)
                        {
                            avCore::Environment::TimeParameters &timeParameters = avCore::GetEnvironment()->m_TimeParameters;

                            if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                                timeParameters.Hour += 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                                timeParameters.Day += 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                               timeParameters.Month += 1;
                            else                                                                    // Increment by one minute
                                timeParameters.Minute += 1;

   
                            return true;
                        }

                        else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract)
                        {
                            avCore::Environment::TimeParameters &timeParameters = avCore::GetEnvironment()->m_TimeParameters;

                            if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                                timeParameters.Hour -= 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                                timeParameters.Day -= 1;
                            else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                                timeParameters.Month -= 1;
                            else                                                                    // Increment by one minute
                                timeParameters.Minute -= 1;

                            return true;
                        }
                        else
                        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_I)
                        {

                            //osgEphemeris::EphemerisData* data = ephem->getEphemerisData();

                            //data->turbidity += 1 ;

                            return true;
                        }
                        else
						if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Page_Up)
						{
							avCore::GetEnvironment()->m_EnvironmentParameters.WindDirection += cg::point_3f(0,.1,0);
							return true;
						}
						else
						if (ea.getKey() == osgGA::GUIEventAdapter:: KEY_Page_Down)
						{
							avCore::GetEnvironment()->m_EnvironmentParameters.WindDirection -= cg::point_3f(0,.1,0);
							return true;
						}
						else
                        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_O)
                        {
                            //osgEphemeris::EphemerisData* data = ephem->getEphemerisData();
                            //// Increment by one minute
                            //data->turbidity -= 1 ;

                            return true;
                        }
                        else
                        if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Rightbracket )
                        { 
                            _intensivity += 0.1;
                            if(_intensivity > 1.0)
                                _intensivity = 1.0;
                            else
                               avCore::GetEnvironment()->m_WeatherParameters.FogDensity = _intensivity; 

                            return true;
                        } else
                        if (ea.getKey()== osgGA::GUIEventAdapter::KEY_Leftbracket)
                        { 
                            _intensivity -= 0.1;
                            if(_intensivity < 0.0) 
                                _intensivity = 0.0;
                            else
                                avCore::GetEnvironment()->m_WeatherParameters.FogDensity = _intensivity;

                            return true;
                        }

        }
        return false;
    } 

private:
    av::weather_params::cloud_type                      _currCloud;
    float                                             _intensivity;
    avSky::Sky *                                      _sky;

};

// cull reflections matrix modificator
class ReflectionCullCallback : public osg::NodeCallback
{
    virtual void operator()( osg::Node * pNode, osg::NodeVisitor * pNV )
    {
        osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
        osg::Group * pNodeAsGroup = static_cast<osg::Group *>(pNode);
        avAssert(pCV && pNodeAsGroup);

        avCore::Prerender * pPrerenderNode = static_cast<avCore::Prerender *>(pNodeAsGroup->getChild(0));
        avAssert(pPrerenderNode);

        FIXME(Need some tides);

        const float fTide = 0.f;

        const osg::Matrixd & mProjection = *pCV->getProjectionMatrix();

        // check underwater status
        osg::Vec3d vEyeLTPPos = pCV->getEyeLocal()/* * avCore::GetCoordinateSystem()->GetLCS2LTPMatrix()*/;
        const bool bUnderWater = vEyeLTPPos.z() < fTide;

        // reverted model view matrix for planar reflections
        osg::Matrixd mModelView = *pCV->getModelViewMatrix();
        if (!bUnderWater)
            mModelView = osg::Matrixd::scale(1.0, 1.0, -1.0) * osg::Matrixd::translate(0.0, 0.0, 2.0 * fTide) * mModelView;

        // extract oblique params
        osg::Matrixd InvMVP = osg::Matrixd::inverse(mModelView * mProjection);
        osg::Vec4d ClipPlane_OS(0.0, 0.0, 1.0, -fTide);
        osg::Vec4d ClipPlane_NDC = InvMVP * ClipPlane_OS;
        ClipPlane_NDC /= abs(ClipPlane_NDC.z()); // normalize such that depth is not scaled
        ClipPlane_NDC.w() -= 1.0;

        // inverse if we are below tide plane
        if (ClipPlane_NDC.z() < 0)
            ClipPlane_NDC *= -1.0;

        // create oblique matrix
        osg::Matrixd suffix;
        suffix(0, 2) = ClipPlane_NDC.x();
        suffix(1, 2) = ClipPlane_NDC.y();
        suffix(2, 2) = ClipPlane_NDC.z();
        suffix(3, 2) = ClipPlane_NDC.w();

        // set projection matrix with oblique clip plane (near plane is set to be clipping plane)
        const osg::Matrixd mNewProjection = mProjection * suffix;
        pPrerenderNode->setProjectionMatrix(mNewProjection);
        pPrerenderNode->setViewMatrix(/*avCore::GetCoordinateSystem()->GetLCS2LTPMatrix() **/ mModelView);

        // go down
        pNV->traverse(*pNode);
    }
};

const osg::Vec3  cameraPos (790,240,30);  //(470,950,100);
// const osg::Vec3  cameraPos (1250.5,-170.5,30);  //(470,950,100);
const osg::Vec3  lookToPos (934.5,-91.5,30);

namespace avGUI {

class  CustomManipulator : public osgGA::FirstPersonManipulator
{
    
protected:
    virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
    {
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left)
        {
            moveRight(-5);
            us.requestRedraw();
            return true;
        }
        else
        if (ea.getKey() == osgGA::GUIEventAdapter:: KEY_Up)
        {
            moveForward(5);
            us.requestRedraw();
            return true;
        }
        else
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right)
        {
            moveRight(5);
            us.requestRedraw();
            return true;
        }
        else
        if (ea.getKey() == osgGA::GUIEventAdapter:: KEY_Down)
        {
            moveForward(-5);
            us.requestRedraw();
            return true;
        }
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F5)
        {
            cg::polar_point_3 p(from_osg_vector3(lookToPos - cameraPos));
            cg::quaternion orien(cpr(-80));
            point_3 forward_dir = orien.rotate_vector(from_osg_vector3(lookToPos - cameraPos)) ;
            setHomePosition(cameraPos, lookToPos + to_osg_vector3(forward_dir), osg::Z_AXIS);
            home(0);
            us.requestRedraw();
            return true;
        }
        else
        if (ea.getKey() == osgGA::GUIEventAdapter:: KEY_F6)
        {
            setHomePosition(cameraPos, lookToPos, osg::Z_AXIS);
            home(0);
            us.requestRedraw();
            return true;
        }
        if (ea.getKey() == osgGA::GUIEventAdapter:: KEY_F7)
        {

            cg::polar_point_3 p(from_osg_vector3(lookToPos - cameraPos));
            cg::quaternion orien(cpr(80));
            point_3 forward_dir = orien.rotate_vector(from_osg_vector3(lookToPos - cameraPos)) ;
            setHomePosition(cameraPos, lookToPos + to_osg_vector3(forward_dir), osg::Z_AXIS);
            home(0);
            us.requestRedraw();
            return true;
        }

        return osgGA::FirstPersonManipulator::handleKeyDown(  ea,  us );
    }
};



    template <class T>
    class GUIEventHandlerImpl : public osgGA::GUIEventHandler
    {
    public:
        GUIEventHandlerImpl( T * object, bool (T::*func)( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv ) ) 
            : _object(object)
            , _func(func)
        {
        }

        virtual bool handle( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv )
        {
            return (_object->*_func)(ea, aa, obj, nv) || osgGA::GUIEventHandler::handle(ea, aa, obj, nv);
        }

    private:
        T * _object;
        bool (T::*_func)( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv );
    };

    template<class T>
    inline GUIEventHandlerImpl<T> * makeGUIEventHandlerImpl( T * object, bool (T::*func)( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv ) )
    {
        return new GUIEventHandlerImpl<T>(object, func);
    }

} // namespace avGUI

struct Scene::_private 
{
	//
	// GUI
	//    

	app::vis_settings_panel_ptr                     _vis_settings_panel;
	app::time_panel_ptr                             _time_panel;
	app::main_window_ptr						    _mw;
	connection_holder                               _conn_holder;

	// uniforms	
//private:
	osg::ref_ptr<osg::Uniform>                      _windTime;

    osg::ref_ptr<avShadow::ShadowTechnique>         _st;  

//private:
	osg::ref_ptr<osg::Node>                         _logo;

	avCore::RandomNumber						    _rndGen;

//private:
	Utils::LoadNodeThread*                          _lnt;
	std::vector<osg::ref_ptr<osg::MatrixTransform>> _mt;

#if !defined(VISUAL_EXPORTS)
	osg::ref_ptr<bi::RigidUpdater>                  _ru;
#endif
	
	osg::ref_ptr<PickHandler>                       _pickHandler; 

//private:
	app::settings_t*                               settings_;
	
	//
	// Something for debug 
	//
    
	osg::ref_ptr<Utils::TrajectoryDrawer>          _trajectory_drawer;
};

osg::ref_ptr<Scene>	 Scene::_scenePtr;


//////////////////////////////////////////////////////////////////////////
bool Scene::Create( osgViewer::Viewer* vw )
{
    _scenePtr = new Scene();

    if ( ! _scenePtr->Initialize( vw ))
    {
        OSG_FATAL << "Failed to initialize scene" ;
        _scenePtr = NULL;
    }

    return _scenePtr.valid();
}
//////////////////////////////////////////////////////////////////////////
void Scene::Release()
{
    if ( _scenePtr.valid() )
    {

        // Release smart pointer with cross references
        _scenePtr->_viewerPtr	= NULL;
        
        gui::releaseCEGUI();

#if !defined(VISUAL_EXPORTS)
        _scenePtr->_p->_ru->stopSession();
#endif
        // Release scene
        _scenePtr = NULL;

        avCore::releaseObjectCache();
    }



    FIXME( Не в сцене этому место задвигать дальше надо)
    osg::discardAllGLObjects(0);
}

//////////////////////////////////////////////////////////////////////////
Scene* Scene::GetInstance()
{
    return _scenePtr.get();
}



//////////////////////////////////////////////////////////////////////////
Scene::Scene()
	: _p (std::make_shared<_private>())
{      
    setName("Scene");

    _loadManager = new avCore::LoadManager();
    addChild(_loadManager);

    _environmentNode = new Group();
    addChild(_environmentNode.get());
    _environmentNode->setName("EnvironmentNode");

    // Common nodes for scene etc.
    _commonNode = new Group();

    osg::StateSet * pCommonStateSet = _commonNode->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");

    addChild(_commonNode.get());

    // Add backface culling to the whole bunch
    osg::StateSet * pSS = getOrCreateStateSet();
    pSS->setNestRenderBins(false);
    pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    // disable alpha writes for whole bunch
    pSS->setAttribute(new osg::ColorMask(true, true, true, false)); 

    setUpdateCallback(Utils::makeNodeCallback(this, &Scene::update));
}

//////////////////////////////////////////////////////////////////////////
Scene::~Scene()
{
}

av::environment_weather* Scene::getEnvWeather() const
{
    return avCore::GetEnvironment();
}

av::ITrajectoryDrawer*    Scene::GetTrajectoryDrawer() const 
{
    return _p->_trajectory_drawer.get();
}

avSky::ISky*  Scene::getSky()
{ 
#ifdef ORIG_EPHEMERIS
    return static_cast<avSky::ISky*>(_ephemerisPtr.get()); 
#else    
    return static_cast<avSky::ISky*>(_skyPtr.get());
#endif
}

namespace
{

osg::Node * createScene()
{
    osg::Group* root = new osg::Group;
    // global sky state-set
    osg::StateSet * pSkyStateSet = root->getOrCreateStateSet();

    pSkyStateSet->setNestRenderBins(false);

    // disable back face culling
    pSkyStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // disable writing to depth buffer
    osg::Depth * pDepth = new osg::Depth;
    //pDepth->setWriteMask(false);
    pSkyStateSet->setAttribute(pDepth);

    osg::Sphere* sphere    = new osg::Sphere( osg::Vec3( 0.f, 0.f, 0.f ), 0.25f );
    osg::ShapeDrawable* sd = new osg::ShapeDrawable( sphere );
    sd->setColor( osg::Vec4( 1.f, 0.f, 0.f, 1.f ) );
    sd->setName( "A nice sphere" );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( sd );

    // Set material for basic lighting and enable depth tests. Else, the sphere
    // will suffer from rendering errors.
    {
        osg::StateSet* stateSet = geode->getOrCreateStateSet();
        osg::Material* material = new osg::Material;

        material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );	

        stateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
    }
    
    root->addChild(geode);
    root->setCullingActive(false);

    return root;
}

}

#define MANIPS

//////////////////////////////////////////////////////////////////////////
bool Scene::Initialize( osgViewer::Viewer* vw)
{
    osg::StateSet* pGlobalStateSet = getOrCreateStateSet();
    osg::StateSet* pCommonStateSet = getCommonNode()->getOrCreateStateSet();
    
    _viewerPtr = vw;            

    //_viewerPtr->setSceneData( this );
   
    //
    // Set up the camera manipulators.
    //

#ifdef MANIPS
    // Use a default camera manipulator

	avGUI::CustomManipulator* manip = new avGUI::CustomManipulator;

    manip->setAcceleration(0);
    manip->setMaxVelocity(1);
    manip->setWheelMovement(10,false);
    //manip->setWheelMovement(0.001,true);
    _viewerPtr->setCameraManipulator(manip);
    //manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
    //manip->home(0);

    //
    // Add event handlers to the viewer
    //

    _p->_pickHandler = new PickHandler();
    _p->settings_ = new  app::settings_t;
    
    _p->settings_->shadow           = true;
    _p->settings_->shadow_for_smoke = true;

    _p->settings_->clouds[0].radius_x = 1000.0f;
    _p->settings_->clouds[0].radius_y = 1000.0f;
    _p->settings_->clouds[0].x = 1000.0f;
    _p->settings_->clouds[0].y = 1000.0f;
    _p->settings_->clouds[0].height = 100.0f;
    _p->settings_->clouds[0].p_type = avWeather::/*PrecipitationRain*/PrecipitationFog;
    _p->settings_->clouds[0].intensity = 0.51f;
    _p->settings_->intensity = 0.0f;

    if(true) _viewerPtr->addEventHandler( 
        gui::createCEGUI( _commonNode, [this]()
        {
            if (!this->_p->_vis_settings_panel )
            {
				auto  pthis = this->_p;
                app::zones_t zones_;
                zones_.push_back(std::make_pair(0,std::wstring(L"Пустая сцена")));
                zones_.push_back(std::make_pair(1,std::wstring(L"Шереметьево")));
                zones_.push_back(std::make_pair(2,std::wstring(L"Сочи")));


				pthis->_mw = app::create_main_win();
				app::menu_ptr fm = pthis->_mw->add_main_menu("File");
				fm->add_string("Exit" , boost::bind(&Scene::onExit,this)); // [&]() { /*exit(0);*/});

				app::menu_ptr vm = pthis->_mw->add_main_menu("View");
				vm->add_string("Lights" , [=]() {  });

				pthis->_mw->set_visible(false);

				pthis->_vis_settings_panel = app::create_vis_settings_panel( zones_, *this->_p->settings_ );

                pthis->_vis_settings_panel->subscribe_zone_changed     (boost::bind(&Scene::onZoneChanged,this,_1));
                pthis->_vis_settings_panel->subscribe_exit_app         (boost::bind(&Scene::onExit,this));
				pthis->_vis_settings_panel->subscribe_set_lights       (boost::bind(&Scene::onSetLights,this,_1));
                pthis->_vis_settings_panel->subscribe_set_shadows      (boost::bind(&Scene::onSetShadows,this,_1, boost::none));
                pthis->_vis_settings_panel->subscribe_set_shadows_part (boost::bind(&Scene::onSetShadows,this,boost::none,_1));
				pthis->_vis_settings_panel->subscribe_set_map          (boost::bind(&Scene::onSetMap,this,_1));
                pthis->_vis_settings_panel->subscribe_set_cloud_param  (boost::bind(&Scene::onSetCloudParams,this,_1));
                pthis->_vis_settings_panel->subscribe_set_global_intensity       (boost::bind(&Scene::onSetGlobalIntensity,this,_1));
                pthis->_vis_settings_panel->set_light(true);

                pthis->_time_panel = app::create_time_panel();

            }
        } )
    );


    _viewerPtr->addEventHandler( new osgViewer::WindowSizeHandler );
    _viewerPtr->addEventHandler( new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()) );
    _viewerPtr->addEventHandler( new osgViewer::StatsHandler );
    _viewerPtr->addEventHandler( _p->_pickHandler );    
    _viewerPtr->addEventHandler( avGUI::makeGUIEventHandlerImpl(this, &Scene::onEvent));
    //_viewerPtr->realize();
	
    addChild(_p->_pickHandler->getOrCreateSelectionBox()); 
#endif


    //
    // Initialize particle engine
    // 

    spark::init();

#ifdef SCREEN_TEXTURE    
    //
    // Screen textures manager node
    //
    _screenTextureManager = new ScreenTextureManager();
    addChild(_screenTextureManager.get());
#endif

    //
    // Create terrain / shadowed scene
    //

    _terrainRoot = createTerrainRoot();
    addChild(_terrainRoot);

#ifdef PPU_TEST
    auto const vp = _viewerPtr->getCamera()->getViewport();
	osg::Camera* cam = _viewerPtr->getCamera();
    // setup an osgPPU pipeline to render the results
    osgPPU::Unit* lastUnit = NULL;
    osgPPU::Processor* ppu = SimpleSSAO::createPipeline(/*vp->width()*/1280, /*vp->height()*/1024, cam, lastUnit, /*showAOMap*/false);
    // create a text unit, which will just print some info
    
    if (lastUnit)
    {
        // OUTPUT ppu
        osgPPU::UnitOut* ppuout = new osgPPU::UnitOut();
        ppuout->setName("PipelineResult");
        ppuout->setInputTextureIndexForViewportReference(-1); // need this here to get viewport from camera
        

        ppuout->setViewport(new osg::Viewport(0,0,1280, 1024));
        lastUnit->addChild(ppuout);
    }

    addChild(ppu);
#endif
    	
	
	
	_p->_windTime   = new osg::Uniform("windTime"  , osg::Vec4(0.0,0.0,0.0,0.0));
	pGlobalStateSet->addUniform(_p->_windTime);
       
    osg::Node* ct =  nullptr;// findFirstNode(_terrainNode,"camera_tower");
    
    


#ifdef MANIPS
    if(ct) 
    {
        osg::Vec3 c = ct->asTransform()->asMatrixTransform()->getMatrix().getTrans();
        manip->setHomePosition(c, osg::Vec3(0,1,0), osg::Z_AXIS);
        manip->home(0);
    }
	else
	{   
	    manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
        manip->home(0);
	}
#endif

    
    avCore::Environment::Create();

    //
    // Create weather
    //
    _weatherPtr = new avWeather::Weather();
    _environmentNode->addChild( _weatherPtr.get() );
    //
    // Cultural and navigational lights manager
    //

#ifdef POINT_LIGHT_MANAGER
    // create it once per scene
    _pointLightsManager = new avLights::PointLightsManager();
    // add point lights manager to common node in the very beginning
    // must be sure weather is already created as we get stateset for local banks notion
    /*_commonNode*/_terrainRoot->insertChild(0, _pointLightsManager.get());
#endif

#if 1 

#ifdef ORIG_EPHEMERIS
    //
    // Create ephemeris
    //                                                                       
    _ephemerisPtr = new avSky::Ephemeris( this , _terrainNode.get() );  

    //
    //  Get or create sunlight
    //
    
    if( _ephemerisPtr->getSunLightSource())
    {
        _ls =_ephemerisPtr->getSunLightSource();  
        if(_terrainRoot) _terrainRoot->addChild(_ls.get());
    }
    else
    {
        _ls = new osg::LightSource;
        _ls->getLight()->setLightNum(0);
        if(_terrainRoot) _terrainRoot->addChild(_ls.get());
        _ephemerisPtr->setSunLightSource(_ls);
    }
    
    
    //if(auto pssm = dynamic_cast<avShadow::ParallelSplitShadowMap*>(_p->_st.get()))
    //        pssm->setUserLight(_ls->getLight());
    
    addChild( _ephemerisPtr.get() );
//////////////////////////////////////////////////////    
    FIXME(Many light sources sm)
#if 0
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;

    osg::Vec4f lightPosition2 (osg::Vec4f(200.0,100.0,50.0,0.0f));
    osg::ref_ptr<osg::Light> myLight2 = new osg::Light;

    myLight2->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.9f,0.9f,0.85f,1.0f));
    myLight2->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    myLight2->setConstantAttenuation(1.0f);

    myLight2->setLightNum(2);
    myLight2->setPosition(lightPosition2);
    ls->setLight(myLight2.get());
    if(_terrainRoot) _terrainRoot->addChild(ls.get());
#endif
/////////////////////////////////////////////////////////////////

    _viewerPtr->addEventHandler(_ephemerisPtr->getEventHandler());

#else  
  
    // Create sky
    //
    _skyPtr = new avSky::Sky( this );
    /*_environmentNode->*/addChild( _skyPtr.get() );
  
    if( _skyPtr->getSunLightSource())
    {
        _ls =_skyPtr->getSunLightSource();  
        if(_terrainRoot) _terrainRoot->addChild(_ls.get());
    }

    _viewerPtr->addEventHandler(new DebugHMIHandler(_skyPtr));

#endif


    LightManager::Create();

    //
    // Dynamic lights manager
    //
    

    addChild(LightManager::GetInstance());

    //
    // Create dynamic lights
    //
    _lights = new Lights();
    _environmentNode->addChild(_lights.get());
   
#if 0   
    //
    // Create weather
    //
    _weatherNode =  new osg::Group;
    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;		
    precipitationEffect->rain(1.0);//snow(0.0);
    precipitationEffect->setWind(osg::Vec3(0.2f,0.2f,0.2f));

    _weatherNode->addChild( precipitationEffect.get() );
    addChild( _weatherNode.get() );
#endif

    //
    // Init physic updater
    //

    createObjects();

    _light_map = createLightMapRenderer(this);
    addChild( _light_map );
	


	//
	// Lights manager test
	//

#if 0
	static const float g_fMaxEnableDeviation = 0.05f;

	// new light group
	avLights::PointLightGroup * pNewGroup = new avLights::PointLightGroup();
	pNewGroup->SetEnableThreshold(g_fMaxEnableDeviation  * _p->_rndGen.random_unit_signed() );
	addChild(pNewGroup);

	std::vector<float> afCommentPulses;
	
	pNewGroup->AddNavigationalLight(
		osg::Vec3(),
		osg::Vec4ub(.99 * 0xFF, .99  * 0xFF, .99 * 0xFF, .99  * 0xFF),
		20, 100000, 0, 60, &afCommentPulses,
		(true) ? 0.f : ((pNewGroup->GetLightsNum() & 0x3F) * 0.1f));
	
	auto nl = new avLights::NavigationalLight;
	addChild(nl);
	nl->SetExhibitionCondition(avLights::NavigationalLight::Always);
#endif


	//
	// Decal Renderer
	//

#ifdef DECAL_RENDERER
    _decal_map = avCore::createDecalRenderer(this);
    addChild( _decal_map );
#endif


    FIXME(140 shaders version needed);

    osgViewer::Viewer::Windows windows;
    _viewerPtr->getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        osg::State *s=(*itr)->getState();
#if GLSL_VERSION > 150
FIXME(Чудеса с Ephemeris)
        s->setUseModelViewAndProjectionUniforms(true);
        s->setUseVertexAttributeAliasing(true);
#endif 
        s->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);
    }

#else
    _ls = new osg::LightSource;
    // _ls->getLight()->setLightNum(0);
    if(_terrainRoot)
        _terrainRoot->addChild(_ls.get());
    
    osg::Vec4f lightPosition2 (osg::Vec4f(-200.0,-100.0,-300.0,0.0f));
    osg::ref_ptr<osg::Light> myLight2 = new osg::Light;
    
    myLight2->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.9f,0.9f,0.85f,1.0f));
    myLight2->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    myLight2->setConstantAttenuation(1.0f);

    myLight2->setLightNum(0);
    myLight2->setPosition(lightPosition2);
    _ls->setLight(myLight2.get());
    _ls->setLocalStateSetModes(osg::StateAttribute::ON); 

    /*_terrainRoot->*/addChild(::createScene());
#endif


#ifdef TEST_WEATHER
	avWeather::Weather * pWeatherNode = avScene::GetScene()->getWeather();

	const avWeather::Weather::WeatherBankIdentifier nID = 666;
	const double dLatitude   = _p->settings_->clouds[0].x;
	const double dLongitude  = _p->settings_->clouds[0].y;
	const float fHeading     = 45;
	const float fEllipseRadX = _p->settings_->clouds[0].radius_x;
	const float fEllipseRadY = _p->settings_->clouds[0].radius_y;
	const float fHeight      = _p->settings_->clouds[0].height;
	const avWeather::PrecipitationType ptType = static_cast<avWeather::PrecipitationType>(_p->settings_->clouds[0].p_type)/*avWeather::PrecipitationRain*/;
	const float fIntensity   = _p->settings_->clouds[0].intensity;
	const float fCentralPortion = 0.75;

	pWeatherNode->UpdateLocalWeatherBank(nID, 
		dLatitude, dLongitude, fHeading, 
		fEllipseRadX, fEllipseRadY, fHeight, 
		ptType, fIntensity, fCentralPortion);

#endif

    //
    // Reflections
    //

    // add creation of main reflection texture
    osg::ref_ptr<avCore::Prerender> pReflFBOGroup = new avCore::Prerender();
    _groupMainReflection = pReflFBOGroup.get();

    // tricky cull for reflections
    osg::ref_ptr<osg::Group> reflectionSubGroup = new osg::Group();
    reflectionSubGroup->addChild(pReflFBOGroup.get());
    reflectionSubGroup->setCullCallback(new ReflectionCullCallback());
    reflectionSubGroup->setName("ReflectionSubGroup");
    _environmentNode->addChild(reflectionSubGroup.get());


    // reflection unit
    getOrCreateStateSet()->addUniform(new osg::Uniform("reflectionTexture", int(BASE_REFL_TEXTURE_UNIT)));
    getOrCreateStateSet()->setTextureAttribute(BASE_REFL_TEXTURE_UNIT, pReflFBOGroup->getTexture());

    _groupMainReflection->addChild(_terrainRoot);
    _groupMainReflection->setName("groupMainReflection");


	//
	// Setup Environment Callback
	//

    avCore::GetEnvironment()->setCallBacks(
        [=](float illum){ if(_p->_st!=0) {_p->_st->setNightMode(illum < 0.8); if(_groupMainReflection.valid()) dynamic_cast<avCore::Prerender*>(_groupMainReflection.get())->setOn(illum < 0.8);  }  }  //  FIXME magic night value
        ,[this](float fog_vr,float fog_exp) {
        BOOST_FOREACH( auto g, this->_lamps)
        {
			auto nag = dynamic_cast<NavAidGroup*>(g.get());
            nag->setMaxVisibleDistance2(fog_vr * fog_vr);
			nag->setFogCoeff(fog_exp);
        }
    }
    );

#if 0
	smoke_sfx_weak_ptr_ = nullptr;
#if 0
	avFx::SmokeFx* smoke = new avFx::SmokeFx;
	smoke_sfx_weak_ptr_ = dynamic_cast<SmokeSfxNode*>(smoke);
	addChild(smoke);
#endif
	fs_sfx_weak_ptr_ = nullptr;
#if 0
	avFx::FoamStreamFx* fs = new avFx::FoamStreamFx;
	fs_sfx_weak_ptr_ = dynamic_cast<FoamStreamSfxNode*>(fs);
	addChild(fs);
#endif
     
    sparks_sfx_weak_ptr = nullptr;
#if 0 
	avFx::SparksFx* spark = new avFx::SparksFx;
	sparks_sfx_weak_ptr = dynamic_cast<SparksSfxNode*>(spark);
	addChild(spark);
#endif

    fd_sfx_weak_ptr_ =0;


#if 0 
	avFx::FrictionDustFx* fd = new avFx::FrictionDustFx;
	fd_sfx_weak_ptr_ = dynamic_cast<FrictionDustSfxNode*>(fd);
	addChild(fd);
#endif

	ld_sfx_weak_ptr_ = nullptr;
#if 0 
	avFx::LandingDustFx* ld = new avFx::LandingDustFx;
	ld_sfx_weak_ptr_ = dynamic_cast<LandingDustSfxNode*>(ld);

	osg::PositionAttitudeTransform* pat;
	pat = new osg::PositionAttitudeTransform;
	pat->setScale(osg::Vec3(1,1,1));
	pat->addChild(ld);
	addChild(pat);
#endif

#endif
   
#ifdef TRAJECTORY_DRAWER
   _p->_trajectory_drawer = new Utils::TrajectoryDrawer(this,Utils::TrajectoryDrawer::LINES);
#endif

#ifdef TEST_ANIMATION
   addChild(tests::createAnimatedObject());
#endif

    return true;
}


osg::Group*  Scene::createTerrainRoot()
{
    osg::Group* tr;
// #undef TEST_SHADOWS_FROM_OSG
#if defined(TEST_SHADOWS_FROM_OSG)

    const int fbo_tex_size = 1024*4;

#ifdef  SHADOW_PSSM
    _p->_st = new avShadow::ParallelSplitShadowMap(NULL,3);
#else
    _p->_st = new avShadow::ViewDependentShadowMap; 
#endif

    tr = new avShadow::ShadowedScene(_p->_st.get());  

    avShadow::ShadowSettings* settings = dynamic_cast<avShadow::ShadowedScene*>(tr)->getShadowSettings();
    
    //settings->setDebugDraw(false);
    
    settings->setShadowMapProjectionHint(avShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);   //ORTHOGRAPHIC_SHADOW_MAP
    settings->setBaseShadowTextureUnit(BASE_SHADOW_TEXTURE_UNIT);
    settings->setMinimumShadowMapNearFarRatio(0.5);
    //settings->setNumShadowMapsPerLight(/*numShadowMaps*/2);
    //settings->setMultipleShadowMapHint(avShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setMultipleShadowMapHint(avShadow::ShadowSettings::CASCADED);
    settings->setTextureSize(osg::Vec2s(fbo_tex_size,fbo_tex_size));
    //settings->setLightNum(2);
    settings->setMaximumShadowMapDistance(1500);
    settings->setShaderHint(avShadow::ShadowSettings::NO_SHADERS);
	//settings->setCastsShadowTraversalMask(cCastsShadowTraversalMask);
	//settings->setReceivesShadowTraversalMask(cReceivesShadowTraversalMask); 
    
    if(auto pssm = dynamic_cast<avShadow::ParallelSplitShadowMap*>(_p->_st.get()))
    {
        pssm->setMaxFarDistance(1024.0);
        double polyoffsetfactor = pssm->getPolygonOffset().x() + 1.1;
        double polyoffsetunit   = pssm->getPolygonOffset().y();
        pssm->setPolygonOffset(osg::Vec2(polyoffsetfactor,polyoffsetunit));
    }

#else
    tr = new osg::Group;
#endif 
    tr->setName("TerrainRoot");
    return tr;

}

void Scene::createObjects()
{

#if !defined(VISUAL_EXPORTS)
    _p->_ru = new bi::RigidUpdater( _terrainRoot->asGroup() 
        ,[&](osg::MatrixTransform* mt){ 
            if(!findFirstNode(mt,"fire"))
            {
                spark::spark_pair_t sp =  spark::create(spark::FIRE,mt);
                sp.first->setName("fire");
                mt->addChild(sp.first);
                this->_viewerPtr->addEventHandler(sp.second);
            }
    }
    );
#endif
    
    if(_p->_ru.valid())
		_p->_ru->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );


    const bool add_vehicles = true;

    if(add_vehicles)
    {
        
    }
#if !defined(VISUAL_EXPORTS)
    if(_p->_ru.valid())
    {
        _viewerPtr->addEventHandler( _p->_ru);

        _p->_conn_holder << _p->_pickHandler->subscribe_choosed_point(boost::bind(&bi::RigidUpdater::handlePointEvent, _p->_ru.get(), _1));
        _p->_conn_holder << _p->_pickHandler->subscribe_selected_node(boost::bind(&bi::RigidUpdater::handleSelectObjectEvent, _p->_ru.get(), _1));

        _p->_conn_holder << _p->_ru->subscribe_selected_object_type(boost::bind(&PickHandler::handleSelectObjectEvent, _p->_pickHandler.get(), _1));
        _p->_ru->setTrajectoryDrawer(new Utils::TrajectoryDrawer(this,Utils::TrajectoryDrawer::LINES));
    }
#endif

}


osg::Node*   Scene::load(std::string path,osg::Node* parent, uint32_t seed, bool async)
{
    using namespace creators;

	auto & mt_ = _p->_mt;
    mt_.push_back(new osg::MatrixTransform); 
    
    LogInfo("Scene::load enter " << path);

    if( path == "smoke" )
    {
        osg::Node* pat =  parent?findFirstNode(parent,"pat",findNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS):nullptr;
        const auto offset =  pat?pat->asTransform()->asPositionAttitudeTransform()->getPosition():osg::Vec3(0.0,0.0,0.0);
       
        osg::Matrix mat; 
        mat.setTrans(-offset/2);
        
        auto mt_offset = new osg::MatrixTransform(mat);
        parent?parent->asGroup()->addChild(mt_offset):nullptr;
        
        spark::spark_pair_t sp_fire =  spark::create(spark::FIRE,parent?mt_offset->asTransform():nullptr);
        sp_fire.first->setName("fire");
        mt_.back()->addChild(sp_fire.first);
        
        //spark::spark_pair_t sp_s =  spark::create(spark::SOMETHING/*,parent?mt_offset->asTransform():nullptr*/);
        //sp_s.first->setName("something");
        //mt_.back()->addChild(sp_s.first);

        //spark::spark_pair_t sp_smoke =  spark::create(spark::SMOKE,parent?mt_offset->asTransform():nullptr);
        //sp_smoke.first->setName("smoke");
        //mt_.back()->addChild(sp_smoke.first);
        
		
		_terrainRoot->asGroup()->addChild(mt_.back());

		//sp_smoke.first->getOrCreateStateSet()->setRenderBinDetails( RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin" );
        sp_fire.first->getOrCreateStateSet()->setRenderBinDetails( RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin" );
 //       sp_s.first->getOrCreateStateSet()->setRenderBinDetails( RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin" );


        //_viewerPtr->addEventHandler(sp_smoke.second);
        _viewerPtr->addEventHandler(sp_fire.second);
        
        //_viewerPtr->addEventHandler(sp_s.second);

        return mt_.back();
    }

	if( path == "sfx//smoke.scg" )
	{
		osg::Node* pat =  parent?findFirstNode(parent,"pat",findNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS):nullptr;
		const auto offset =  pat?pat->asTransform()->asPositionAttitudeTransform()->getPosition():osg::Vec3(0.0,0.0,0.0);

		osg::Matrix mat; 
		mat.setTrans(-offset/2);

		auto mt_offset = new osg::MatrixTransform(mat);
		parent?parent->asGroup()->addChild(mt_offset):nullptr;

		avFx::SmokeFx* fs = new avFx::SmokeFx;

		// fs->setTrackNode(parent);

		mt_.back()->addChild(fs);
		_terrainRoot/*parent*/->asGroup()->addChild(mt_.back());

		return mt_.back();
	}

    if( path == "sfx//landing_dust.scg" )
    {
        osg::Node* pat =  parent?findFirstNode(parent,"pat",findNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS):nullptr;
        const auto offset =  pat?pat->asTransform()->asPositionAttitudeTransform()->getPosition():osg::Vec3(0.0,0.0,0.0);

        osg::Matrix mat; 
        mat.setTrans(-offset/2);

        auto mt_offset = new osg::MatrixTransform(mat);
        parent?parent->asGroup()->addChild(mt_offset):nullptr;

        avFx::LandingDustFx* fs = new avFx::LandingDustFx;

        mt_.back()->addChild(fs);
        _terrainRoot->asGroup()->addChild(mt_.back());

        return mt_.back();
    }

	if( path == "sfx//foam_stream.scg" )
	{
		osg::Node* pat =  parent?findFirstNode(parent,"pat",findNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS):nullptr;
		const auto offset =  pat?pat->asTransform()->asPositionAttitudeTransform()->getPosition():osg::Vec3(0.0,0.0,0.0);

		osg::Matrix mat; 
		mat.setTrans(-offset/2);

		auto mt_offset = new osg::MatrixTransform(mat);
		parent?parent->asGroup()->addChild(mt_offset):nullptr;

		avFx::FoamStreamFx* fs = new avFx::FoamStreamFx;

#ifdef TRACKNODE 		
		fs->setTrackNode(parent);
		mt_.back()->addChild(fs);
		_terrainRoot->asGroup()->addChild(mt_.back());
#else
        mt_.back()->addChild(fs);
        parent->asGroup()->addChild(mt_.back());
#endif

		return mt_.back();
	}

    if( path == "text_label.scg" )
    {
        osg::Node* pat =  parent?findFirstNode(parent,"pat",findNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS):nullptr;
        const auto offset =  pat?pat->asTransform()->asPositionAttitudeTransform()->getPosition():osg::Vec3(0.0,0.0,0.0);
        
        osg::MatrixTransform* root = nullptr;
        
        osg::Node* phys_ctrl = findFirstNode(parent,"phys_ctrl",findNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS);
		
		if(phys_ctrl)
		{
			auto label = findFirstNode(phys_ctrl,"text_label");
        
			if( label == nullptr)
			{
				const osg::Quat quat0(osg::inDegrees(90.0f), osg::X_AXIS,                      
									  osg::inDegrees(0.f)  , osg::Y_AXIS,
									  osg::inDegrees(90.f) , osg::Z_AXIS ); 
            
				const osg::Quat quat1(osg::inDegrees(90.0f)  , osg::X_AXIS,                      
									  osg::inDegrees(0.f)    , osg::Y_AXIS,
									  osg::inDegrees(-90.f)  , osg::Z_AXIS ); 
            
				// double radius = phys_ctrl->computeBound().radius();
				osg::ComputeBoundsVisitor cbv;
				parent->accept( cbv );
				const osg::BoundingBox& bb = cbv.getBoundingBox();

				osg::Matrix mat; 
				mat.setTrans(-offset/2);

				root = new osg::MatrixTransform(mat);
				root->setName("root");
				phys_ctrl?phys_ctrl->asGroup()->addChild(root):nullptr;
        
				std::string timesFont("fonts/times.ttf");

				osg::ref_ptr<osgText::Text> updateText = new osgText::Text;
				updateText->setDataVariance(osg::Object::DYNAMIC);

				std::string font("fonts/times.ttf");
				//std::string font("fonts/arial.ttf");

				updateText->setFont(font);
				updateText->setFontResolution(110,120);
				//updateText->setAlignment(osgText::Text::RIGHT_CENTER);
				//updateText->setAxisAlignment(osgText::Text::XZ_PLANE);
				//updateText->setCharacterSize((bb.zMax()-bb.zMin())*1.0f);
				//updateText->setPosition(bb.center()-osg::Vec3((bb.xMax()-bb.xMin()),-(bb.yMax()-bb.yMin())*0.5f,(bb.zMax()-bb.zMin())*0.1f));
				//updateText->setColor(osg::Vec4(0.37f,0.48f,0.67f,1.0f)); // Neil's original OSG colour
				updateText->setColor(osg::Vec4(0.20f,0.45f,0.60f,1.0f)); 


				osg::Geode* geode = new osg::Geode();
				osg::StateSet* stateset = geode->getOrCreateStateSet();
				stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
				stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
				geode->setName("text_label");
				geode->addDrawable( updateText );
        
				updateText->setName("text_label");
				updateText->setCharacterSize(1.8f);
				updateText->setFont(timesFont);
				updateText->setColor(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
				updateText->setText("The label");
				updateText->setPosition(osg::Vec3(0,0,(bb.zMax() - bb.zMin())));
				updateText->setRotation(quat0);
				updateText->setAlignment(osgText::TextBase::CENTER_TOP);
				auto cpText = osg::clone(updateText.get(), osg::CopyOp::DEEP_COPY_ALL);
				cpText->setRotation(quat1);
				geode->addDrawable( cpText );

				root->addChild(geode);
			}
			else
			{
			   root = label->getParent(0)->asTransform()->asMatrixTransform();
			}
		}

        return root;
    }

    if (path == "adler" || path == "sheremetyevo" || path == "minsk" || path == "lipetsk" || path == "eisk" || path == "vnukovo")
    {
        //assert(_terrainRoot->removeChild(_terrainNode));
        //_terrainNode.release();
        _terrainNode =  new avTerrain::Terrain (this);
        _terrainNode->Create(path);
        
        _terrainRoot->asGroup()->addChild(_terrainNode);
		
#ifdef DECAL_RENDERER
        setupDecals(path);
#endif

#if 0 
        load("su_27",_terrainRoot, 15000, false);
        load("mi_24",_terrainRoot, 15000, false);
        load("mig_29",_terrainRoot, 15000, false); 
#endif
        // load("an_26",_terrainRoot, 15000, false);
        // load("trees",_terrainRoot, 15000);
        // load("ka_50",_terrainRoot, 15000, false);

         /*_commonNode*//*this*/_terrainRoot->setCullCallback(new DynamicLightsObjectCull(/*GlobalInfluence*/LocalInfluence));

        return _terrainNode;
    }


    {
        mt_.back()->setName("phys_ctrl");
        mt_.back()->setUserValue("id",seed);
        mt_.back()->getOrCreateStateSet()->setRenderBinDetails( RENDER_BIN_SCENE, "DepthSortedBin" );
    }

    high_res_timer hr_timer;

	
	auto sig = [&](uint32_t seed)->void {object_loaded_signal_(seed);};

    auto  wf =  [this](uint32_t seed, std::string path, osg::MatrixTransform* mt, bool async)->osg::Node* {
    
		auto & mt_ = _p->_mt;
    bool clone = true;

    using namespace creators;
    
    avCore::Object* obj = avCore::createObject(path, clone);
    
    if( path == "su_27" )
    {
       osg::Matrix trMatrix;
       trMatrix.setTrans(osg::Vec3d(971,50,0));
       trMatrix.setRotate(osg::Quat(osg::inDegrees(139.0), osg::Z_AXIS));

       mt_.back()->setMatrix(trMatrix);
    }
    
    if( path == "mig_29" )
    {
        osg::Matrix trMatrix;
        trMatrix.setTrans(osg::Vec3d(1000,25,0));
        trMatrix.setRotate(osg::Quat(osg::inDegrees(139.0), osg::Z_AXIS));

        mt_.back()->setMatrix(trMatrix);
    }

    if(obj)
    {
        if(obj->hwInstanced())
        {
             obj->parentMainInstancedNode(_terrainRoot->asGroup());
        }

        osg::Node* obj_node = obj->getOrCreateNode();
        
        mt->addChild( obj_node );

		osg::Node* root =  findFirstNode(obj_node,"root"); 
        if(root!=nullptr) root->setUserValue("id",seed);
        
        if(mt!=nullptr)
        {
            
            osg::Node* ld  =  findFirstNode(mt,"landing_lamp" ,findNodeVisitor::exact);
            if(ld==nullptr)
                ld =  findFirstNode(mt,"landing_lamp_r" ,findNodeVisitor::exact);
			osg::Node* ld1 =  findFirstNode(mt,"landing_lamp1",findNodeVisitor::exact);
            if(ld1==nullptr)
                ld1 =  findFirstNode(mt,"landing_lamp_l" ,findNodeVisitor::exact);

			osg::Node* sl  =  findFirstNode(mt,"steering_lamp",findNodeVisitor::not_exact);
            osg::Node* pat =  findFirstNode(mt,"pat"          ,findNodeVisitor::not_exact);
            osg::Node* hd  =  findFirstNode(mt,"headlight"    ,findNodeVisitor::not_exact);

            const auto offset =  pat?pat->asTransform()->asPositionAttitudeTransform()->getPosition():osg::Vec3d(0.0,0.0,0.0);

            if(sl)
            {
                avScene::LightManager::Light data;
				data.transform       = mt;  
				data.spotFalloff     = cg::range_2f(osg::DegreesToRadians(25.f), osg::DegreesToRadians(33.f));
				data.distanceFalloff = cg::range_2f(75.f, 140.f);
				data.color.r = 0.92f;
                data.color.g = 0.92f;
                data.color.b = 0.85f;
                FIXME(  Damned offset  );
                data.position =  from_osg_vector3(sl->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);

                osg::Quat      rot   = sl->asTransform()->asMatrixTransform()->getMatrix().getRotate();
                cg::quaternion orien = from_osg_quat(rot);
                cg::cpr        cr    = orien.cpr(); 

                const float heading = osg::DegreesToRadians(cr.course);
                const float pitch   = osg::DegreesToRadians(/*cr.pitch*/15.f);

                data.direction = set_direction(pitch, heading);
                data.active = true;
//#ifndef ASYNC_OBJECT_LOADING
                avScene::LightManager::GetInstance()->addLight(data);
//#endif
				auto nl = new avLights::NavigationalLight;
				sl->asGroup()->addChild(nl);
                nl->SetExhibitionCondition(avLights::NavigationalLight::NightTimeOnly);
                nl->SetColor(osg::Vec4ub(245,250,255,255));
                nl->SetSectorRange(-90,90);
                nl->SetSize(0.75);
            }

			if(ld)
			{
				avScene::LightManager::Light data;
				data.transform       = mt;  
				data.spotFalloff     = cg::range_2f(osg::DegreesToRadians(15.f), osg::DegreesToRadians(23.f));
				data.distanceFalloff = cg::range_2f(55.f, 100.f);
                data.color.r = 0.82f;
                data.color.g = 0.82f;
                data.color.b = 0.75f;

				data.position =  from_osg_vector3(ld->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);

				osg::Quat      rot   = ld->asTransform()->asMatrixTransform()->getMatrix().getRotate();
				cg::quaternion orien = from_osg_quat(rot);
				cg::cpr        cr    = orien.cpr(); 

				const float heading = osg::DegreesToRadians(cr.course);
				const float pitch   = osg::DegreesToRadians(cr.pitch/*15.f*/);

				data.direction = set_direction(pitch, heading);
				data.active = true;
				//#ifndef ASYNC_OBJECT_LOADING
				avScene::LightManager::GetInstance()->addLight(data);
				//#endif
				auto nl = new avLights::NavigationalLight;
				ld->asGroup()->addChild(nl);
				nl->SetExhibitionCondition(avLights::NavigationalLight::NightTimeOnly);
				nl->SetColor(osg::Vec4ub(245,250,255,255));
				nl->SetSectorRange(-90,90);
                nl->SetSize(0.75);
			}

			if(ld1)
			{
				avScene::LightManager::Light data;
				data.transform       = mt;  
				data.spotFalloff     = cg::range_2f(osg::DegreesToRadians(15.f), osg::DegreesToRadians(23.f));
				data.distanceFalloff = cg::range_2f(55.f, 100.f);
				data.color.r = 0.82f;
				data.color.g = 0.82f;
				data.color.b = 0.75f;

				data.position =  from_osg_vector3(ld1->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);

				osg::Quat      rot   = ld1->asTransform()->asMatrixTransform()->getMatrix().getRotate();
				cg::quaternion orien = from_osg_quat(rot);
				cg::cpr        cr    = orien.cpr(); 

				const float heading = osg::DegreesToRadians(cr.course);
				const float pitch   = osg::DegreesToRadians(cr.pitch/*15.f*/);

				data.direction = set_direction(pitch, heading);
				data.active = true;
				//#ifndef ASYNC_OBJECT_LOADING
				avScene::LightManager::GetInstance()->addLight(data);
				//#endif
				
                auto nl = new avLights::NavigationalLight;
				ld1->asGroup()->addChild(nl);
                nl->SetExhibitionCondition(avLights::NavigationalLight::NightTimeOnly);
                nl->SetColor(osg::Vec4ub(245,250,255,255));
                nl->SetSectorRange(-90,90);
                nl->SetSize(0.75);
			}

            if(hd)
            {
                avScene::LightManager::Light data;
                data.transform  = mt;  
                data.spotFalloff = cg::range_2f(osg::DegreesToRadians(15.f), osg::DegreesToRadians(32.f));  // FIXME градусы-радианы 
                data.distanceFalloff = cg::range_2f(15.f, 70.f);
                data.color.r = 0.55f;
                data.color.g = 0.55f;
                data.color.b = 0.5f;
                FIXME(  Damned offset  );
                data.position =  from_osg_vector3(hd->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);

                osg::Quat      rot   = hd->asTransform()->asMatrixTransform()->getMatrix().getRotate();
                cg::quaternion orien = from_osg_quat(rot);
                cg::cpr        cr    = orien.cpr(); 

                const float heading = osg::DegreesToRadians(cr.course);
                const float pitch = osg::DegreesToRadians(/*cr.pitch*/15.f);

                data.direction = set_direction(pitch, heading);
                data.active = true;
//#ifndef ASYNC_OBJECT_LOADING
                avScene::LightManager::GetInstance()->addLight(data);
//#endif
            }

            findNodeVisitor::nodeNamesList list_name;
            NavAidGroup*  obj_light  =  new NavAidGroup; 

            const char* names[] =
            {
                "port",
                "starboard",
                "tail",
                "steering_lamp",
                "strobe_",
                "landing_lamp",
                "back_tail",
                "headlight"
                // "navaid_",
            };

            for(int i=0; i<sizeof(names)/sizeof(names[0]);++i)
            {
                list_name.push_back(names[i]);
            }

            findNodeVisitor findNodes(list_name,findNodeVisitor::not_exact); 
            if(root) root->accept(findNodes);

            const findNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();

            auto shift_phase = cg::rand(cg::range_2(0, 255));

            osgSim::Sector* sector = new osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));

            for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
            {
                osgSim::LightPoint pnt;
                bool need_to_add = false;
                avScene::LightManager::Light data;

                if((*it)->getName() == "tail")
                { 
                    pnt._color      = creators::white_color * 0.01f;
                    need_to_add     = true;
                }
                
                if((*it)->getName() == "back_tail")
                { 
                    avScene::LightManager::Light data;

                    data.color.r = 0.92f;
                    data.color.g = 0.92f;
                    data.color.b = 0.85f;

                    data.transform  = mt;  
                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(4.f, 5.f);
                    FIXME( Damned offset )
                    data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);
                    data.active = true;
                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f);
                    
                    data.high_priority = true;
                    data.normal_coeff  = 1.0;   // XXX
                    data.direction = set_direction(pitch, heading);

                    avScene::LightManager::GetInstance()->addLight(data);
                }

                if((*it)->getName() == "port")
                {   
                    pnt._color      = green_color * 0.01f;
                    need_to_add     = true;
                    pnt._sector = sector;

                    data.transform  = mt;  
                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(1.5f, 10.f);
                    FIXME( Damned offset )
                    data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);
					data.lm_only = true;
                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f);

					data.direction = set_direction(pitch, heading);

                }

                if((*it)->getName() == "starboard") 
                {
                    pnt._color = red_color * 0.01f ;
                    need_to_add     = true;
                    pnt._sector = sector;

                    data.transform  = mt;  
                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(1.5f, 10.f);
                    FIXME( Damned offset )
                    data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset); 
					data.lm_only = true;

                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f);

                    data.direction = set_direction(pitch, heading);
                }


                if(boost::starts_with((*it)->getName(), "strobe_")) 
                {
                    pnt._color  = white_color * 0.01f;
                    pnt._blinkSequence = new osgSim::BlinkSequence;
                    pnt._blinkSequence->addPulse( 0.05,
                        osg::Vec4( 1., 1., 1., 1. ) );

                    pnt._blinkSequence->addPulse( 1.5,
                        osg::Vec4( 0., 0., 0., 0. ) );

                    pnt._sector = new osgSim::AzimSector(-osg::inDegrees(170.0),-osg::inDegrees(10.0),osg::inDegrees(90.0));

                    pnt._blinkSequence->setPhaseShift(shift_phase);
                    need_to_add     = true;

                    data.transform  = mt;  
                    data.spotFalloff = cg::range_2f();
                    data.distanceFalloff = cg::range_2f(3.5f, 15.f);//cg::range_2f(3.5f, 40.f);

                    FIXME( Damned offset )
                    data.position =  from_osg_vector3((*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans() + offset);

                    const float heading = osg::DegreesToRadians(0.f);
                    const float pitch   = osg::DegreesToRadians(0.f);

                    data.direction = set_direction(pitch, heading);
                }

                pnt._position = (*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans();
                pnt._radius = 0.2f;
                //И чего тут делать с огнями и колбеками
                FIXME( Исправить структуру под mt)
//#ifndef ASYNC_OBJECT_LOADING
                if(need_to_add)
                {
                    obj_light->addLight(pnt, data);
                }
//#endif
            }

            //И чего тут делать с огнями и колбеками
            FIXME( Исправить структуру под mt)
//#ifndef ASYNC_OBJECT_LOADING
                if(wln_list.size()>0)
                    root->asGroup()->addChild(obj_light);
//#endif

        }

//#ifndef ASYNC_OBJECT_LOADING
        if(!async)
            _terrainRoot->asGroup()->addChild(mt);
//#endif

#if 0
		FIXME("Жесть с анимацией, кто на ком стоял")
	    using namespace avAnimation;
		if(path=="crow")
		{
			AnimationManagerFinder finder;
			mt->accept(finder);
			if(finder._am.valid())
			{
				SetupRigGeometry switcher(true, *mt);
#ifdef ASYNC_OBJECT_LOADING
				mt->setUpdateCallback(finder._am.get());
#endif

			}
		}
#endif

#ifndef ASYNC_OBJECT_LOADING
        object_loaded_signal_(seed);
#endif  

		return mt;

    }

	return nullptr;
 };



#ifdef ASYNC_OBJECT_LOADING
    if(async)
		dynamic_cast<avCore::LoadManager*>(_loadManager.get())->load(mt_.back(), boost::bind<osg::Node*>( wf, seed,path,mt_.back().get(), async),boost::bind<void>(sig, seed));
	else
		wf(seed, 
		path, 
		mt_.back().get(),
		async);
#else
    wf(seed, path,mt_.back().get(),async);
#endif
    
    OSG_WARN << "Scene::load: " << hr_timer.set_point() << "\n";

    LogInfo("Scene::load exit " << path);



    return mt_.back();
}

void   Scene::onSetMap(float val)
{
	_terrainNode->setGrassMapFactor(val);
}

void   Scene::onSetGlobalIntensity(float val)
{
    avCore::GetEnvironment()->m_WeatherParameters.RainDensity = val;
}

void   Scene::onSetCloudParams(const app::cloud_params_t& s)
{
    avWeather::Weather * pWeatherNode = avScene::GetScene()->getWeather();

    pWeatherNode->UpdateLocalWeatherBank(666, 
        s.x, s.y, /*fHeading*/0, 
        s.radius_x, s.radius_y, s.height, 
        /*ptType*/avWeather::/*PrecipitationRain*/PrecipitationFog , s.intensity, /*fCentralPortion*/1.0);
}


void   Scene::onSetLights(bool on)
{
	if (_lights.valid())
    {
        _lights->setNodeMask(on?0xffffffff:0);
        LightManager::GetInstance()->setNodeMask(on?REFLECTION_MASK:0);
    }
}

void   Scene::onSetShadows(const optional<bool>& on, const optional<bool>& on_part )
{
    if (on != boost::none)
    {
       _p->_st->enableShadows(*on);
    }

    if (on_part != boost::none)
    {
        _p->_st->enableParticleShadows(*on_part);
    }

}

void   Scene::onZoneChanged(int zone)
{
    const char* scene_name[] = {"empty","adler","sheremetyevo","minsk"};
    
    // zone_to_reload_ = scene_name[zone];

    //_terrainRoot->removeChild(_terrainNode);
    //_terrainNode.release();
    //_terrainNode =  new avTerrain::Terrain (_terrainRoot);
    //_terrainNode->create(scene_name[zone]);

}

bool Scene::onEvent( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & /*aa*/, osg::Object * /*obj*/, osg::NodeVisitor * /*nv*/ )
{
    if (ea.getHandled() || ea.getEventType() != osgGA::GUIEventAdapter::KEYUP)
        return false;

    const int key = ea.getKey();

    if (key == osgGA::GUIEventAdapter::KEY_Escape)
    {
		if(_p->_mw) 
			_p->_mw->set_visible(!_p->_mw->visible());

        return true;
    }

    return false;
}

void Scene::onExit()
{
    _viewerPtr->setDone(true);
}

void Scene::setHomePosition(const osg::Vec3d& eye, const osg::Vec3d& center)
{
    osgGA::CameraManipulator* manip = _viewerPtr->getCameraManipulator();
    if(manip)
    { 
        manip->setHomePosition(eye, center, osg::Z_AXIS);
        manip->home(0);
    }
}

// update pass
void Scene::update( osg::NodeVisitor * nv )
{
      const avCore::Environment::EnvironmentParameters & cEnvironmentParameters= avCore::GetEnvironment()->GetEnvironmentParameters();
	  const double lt = nv->getFrameStamp()->getSimulationTime();
      
      if(_p->_time_panel)
      {
          _p->_time_panel->set_time(lt * 1000.f);
      }

      const unsigned base_hour = 10;
      const unsigned time_coeff   = 1/*500*/; 
      const double et = lt * time_coeff;

      avCore::Environment::TimeParameters & vTime = avCore::GetEnvironment()->GetTimeParameters();
      bool bsetup =  (vTime.Second != unsigned(et) % 60) || (vTime.Minute != unsigned(et / 60.0)  % 60); 
      
      vTime.Hour   = base_hour + unsigned(et / 3600.0)  % 24;
      vTime.Minute = unsigned(et / 60.0)  % 60;
      vTime.Second = unsigned(et) % 60;

      if(bsetup && _ephemerisPtr)
        _ephemerisPtr->setTime();


#if 0
	  if (smoke_sfx_weak_ptr_)
	  {
		  auto const intensity = 4.0f;

		  smoke_sfx_weak_ptr_->setFactor(intensity * cg::clamp(0., /*smoke_end_duration_*/10., 1., 0.)(cg::mod(_viewerPtr->getFrameStamp()->getSimulationTime(),5)));
		  smoke_sfx_weak_ptr_->setIntensity(intensity * 2);
	      
	  }

	  if(sparks_sfx_weak_ptr)
	  {
		  sparks_sfx_weak_ptr->setEmitterWorldSpeed(point_3f(20.f,20.f,20.f));
		  sparks_sfx_weak_ptr->setContactFlag(true);
	  }

	  if(fd_sfx_weak_ptr_)
	  {
		  fd_sfx_weak_ptr_->setEmitterWorldSpeed(point_3f(20.f,20.f,20.f));
		  fd_sfx_weak_ptr_->setContactFlag(true);
	  }

	  if(ld_sfx_weak_ptr_ && cg::mod(int(lt),5) == 0 )
	  {
		  ld_sfx_weak_ptr_->makeContactDust(lt, cg::point_3f(0, 0, 0)/*cg::point_3f(-20, 110, 0)*/, cg::point_3f(-60.f, -10.f, 0.f));
	  }

	  if (fs_sfx_weak_ptr_)
	  {
		  auto const intensity = 1.0f;

		  fs_sfx_weak_ptr_->setFactor(intensity * cg::clamp(0., /*smoke_end_duration_*/10., 1., 0.)(cg::mod(_viewerPtr->getFrameStamp()->getSimulationTime(),2.5)));
		  fs_sfx_weak_ptr_->setIntensity(intensity * 120);
		  //fs_sfx_weak_ptr_->setEmitterWorldSpeed(cg::point_3f(20, 10, 20) * 2);
	  }	  
#endif


	  FIXME( "Ну и нафиг оно в cg?" );
	  FIXME( "А третья координата?" );
	  cg::point_3f wind = cEnvironmentParameters.WindSpeed * cEnvironmentParameters.WindDirection;
	  _p->_windTime->set(osg::Vec4(wind.x,wind.y,lt,0.0));
	  
}

bool Scene::PreUpdate()
{
    bool ret = true;
    if (auto lm = dynamic_cast<avCore::LoadManager*>(_loadManager.get()))
    {
        ret &= lm->PreUpdate();
    }

    return ret;
}

inline void fill_parking (avCore::IDecalRendererPtr decal_map, const std::vector<cg::point_2f>& ptemplate , cg::point_2f pos ,float course, float coeff_size)
{
    std::vector<cg::point_2f> pnts_array;
    pnts_array.resize(ptemplate.size()); 
    cg::transform_3f tr(cg::as_translation(pos),cg::rotation_2f(/*180*/),cg::scale_2f(/*3.0*/));
    std::transform(ptemplate.begin(), ptemplate.end(), pnts_array.begin(), [=]( const cg::point_2f & val)->cg::point_2f { return cg::point_2f(val.x * 10.f, val.y * 11.f)  * 1.f / (coeff_size) * cg::rotation_2f(-course) * tr;});
    decal_map->AddPolyline(pnts_array, cg::colorf(0.0f, 0.0f, 0.8f), 0.3f );
};

void Scene::setupDecals(const std::string& scene) 
{
    if(scene!="eisk")
        return;
    
    if(!_decal_map)
        return;

#if 0   // White T
    std::vector<cg::point_2f> pnts;
    pnts.emplace_back(cg::point_2f(0.f,0.f));
    pnts.emplace_back(cg::point_2f(0.f, 10.f));
    pnts.emplace_back(cg::point_2f(5.f, 10.f));
    pnts.emplace_back(cg::point_2f(10.f, 10.f));
    pnts.emplace_back(cg::point_2f(0.f, 10.f));
    pnts.emplace_back(cg::point_2f(-5.f, 10.f));   
    
    
    std::array<cg::transform_3f,10> trs;
    trs[0] = cg::as_translation(cg::point_2f(100,100));
    trs[1] = cg::as_translation(cg::point_2f(150,150));
    trs[2] = cg::as_translation(cg::point_2f(200,300));
    trs[3] = cg::as_translation(cg::point_2f(210,150));
    trs[4] = cg::as_translation(cg::point_2f(50,60));
    trs[5] = cg::as_translation(cg::point_2f(0,0));
    trs[6] = cg::as_translation(cg::point_2f(20,0));
    trs[7] = cg::as_translation(cg::point_2f(40,0));
    trs[8] = cg::as_translation(cg::point_2f(0,40));
    trs[9] = cg::as_translation(cg::point_2f(0,80));


    std::array< std::vector<cg::point_2f>,10> pnts_array;
    for (int i = 0;i < pnts_array.size(); ++i )
    {
        pnts_array[i].resize(pnts.size());
        std::transform(pnts.begin(), pnts.end(), pnts_array[i].begin(), [=]( const cg::point_2f & val)->cg::point_2f { return val  * trs[i];});
        _decal_map->AddPolyline(pnts_array[i], cg::colorf(0.80f, 0.80f, 0.80f), 0.5f );
    }
#endif
	
		std::vector<cg::point_2f> pnts_red;
		pnts_red.emplace_back(cg::point_2f(0.f,-2.f));
		pnts_red.emplace_back(cg::point_2f(-1.f, -2.f));
		pnts_red.emplace_back(cg::point_2f(-2.f, -1.f));
		pnts_red.emplace_back(cg::point_2f(-2.f, 1.f));
		pnts_red.emplace_back(cg::point_2f(-1.f, 2.f));
		pnts_red.emplace_back(cg::point_2f(0.f, 2.f));
		pnts_red.emplace_back(cg::point_2f(1.f, 2.f));
		pnts_red.emplace_back(cg::point_2f(2.f, 1.f));
		pnts_red.emplace_back(cg::point_2f(2.f, -1.f)); 
		pnts_red.emplace_back(cg::point_2f(1.f, -2.f));
		pnts_red.emplace_back(cg::point_2f(0.f, -2.f)); 
    {
		std::vector<cg::point_2f> pnts_array;
		pnts_array.resize(pnts_red.size()); // point_3(201,392,0),cg::cpr(173)
		cg::transform_3f tr(cg::as_translation(cg::point_2f(201,392)),cg::rotation_2f(/*180*/),cg::scale_2f(/*3.0*/));
		//tr = cg::as_translation(cg::point_2f(201,392));
		std::transform(pnts_red.begin(), pnts_red.end(), pnts_array.begin(), [=]( const cg::point_2f & val)->cg::point_2f { return cg::point_2f(val.x * 10.f, val.y * 11.f) * cg::rotation_2f(-173) * tr;});
		_decal_map->AddPolyline(pnts_array, cg::colorf(0.80f, 0.0f, 0.0f), 0.3f );
	}

	{
		std::vector<cg::point_2f> pnts_array;
		pnts_array.resize(pnts_red.size()); // point_3(201,392,0),cg::cpr(173)
		cg::transform_3f tr(cg::as_translation(cg::point_2f(201,392)),cg::rotation_2f(/*180*/),cg::scale_2f(/*3.0*/));
		//tr = cg::as_translation(cg::point_2f(201,392));
		std::transform(pnts_red.begin(), pnts_red.end(), pnts_array.begin(), [=]( const cg::point_2f & val)->cg::point_2f { return cg::point_2f(val.x * 10.f, val.y * 11.f) * 1.f / (1.7436 * 1.2823) * cg::rotation_2f(-173) * tr;});
		_decal_map->AddPolyline(pnts_array, cg::colorf(0.00f, 0.80f, 0.0f), 0.3f );
	}

	{
		std::vector<cg::point_2f> pnts_array;
		pnts_array.resize(pnts_red.size()); // point_3(201,392,0),cg::cpr(173)
		cg::transform_3f tr(cg::as_translation(cg::point_2f(201,392)),cg::rotation_2f(/*180*/),cg::scale_2f(/*3.0*/));
		//tr = cg::as_translation(cg::point_2f(201,392));
		std::transform(pnts_red.begin(), pnts_red.end(), pnts_array.begin(), [=]( const cg::point_2f & val)->cg::point_2f { return cg::point_2f(val.x * 10.f, val.y * 11.f)  * 1.f / (1.2823) * cg::rotation_2f(-173) * tr;});
		_decal_map->AddPolyline(pnts_array, cg::colorf(0.80f, 0.80f, 0.0f), 0.3f );
	}

    const std::string parking [] = {
    "13 ЦЗТ 0.043 0.370 173 1",
    "14 ЦЗТ 0.079 0.374 173 1",
    "15 ЦЗТ 0.115 0.378 173 1",
    "16 ЦЗТ 0.156 0.387 173 2",
    "17 ЦЗТ 0.201 0.392 173 2",
    "18 ЦЗТ 0.245 0.398 173 2",
    "19 ЦЗТ 0.286 0.400 173 1",
    "20 ЦЗТ 0.322 0.404 173 1",
    "21 ЦЗТ 0.357 0.408 173 1",
    "22 ЦЗТ 0.394 0.413 173 1",
    "23 ЦЗТ 0.443 0.418 173 1",
    "24 ЦЗТ 0.479 0.423 173 1",
    "25 ЦЗТ 0.514 0.428 173 1",
    "26 ЦЗТ 0.551 0.432 173 1",
    "27 ЦЗТ 0.587 0.437 173 1",
    "28 ЦЗТ 0.622 0.437 173 0",
    "29 ЦЗТ 0.647 0.440 173 0",
    "30 ЦЗТ 0.672 0.443 173 0",
    "31 ЦЗТ 0.696 0.446 173 0",
    "32 ЦЗТ 0.721 0.449 173 0",
    "33 ЦЗТ 0.746 0.451 173 0",
    };

    const size_t psize = sizeof(parking) / sizeof(parking[0]);
    for( int i =0; i < psize; ++i )
    {
        std::vector<std::string> values_;
        boost::split(values_, parking[i], boost::is_any_of(" \t="), boost::token_compress_on);
        auto ptype = boost::lexical_cast<int>(values_[5]);
        fill_parking(_decal_map, pnts_red,1000.f * point_2f(boost::lexical_cast<float>(values_[2]),boost::lexical_cast<float>(values_[3])),boost::lexical_cast<int>(values_[4]), (ptype==0) ? 1.7436 * 1.2823: ((ptype==1)? 1.2823:1));
    }

	/// 1,7436 mid to low
    /// 1,2823 big to mid

	const std::string geoms_names [] = {
		"KraiWhite",
		"MarkingVPP",
		"Marking02",
		"NiytkaRazmetka2",
		"NiytkaRazmetka",
		"NitkaState",
		"marking01",
		"Tr01White",
		"Tr02White01",
		"Tr03Bel01",
		"State01Red1",
		"State03Red",
		"State02Red",
		"StopRazmetka",

	};

	const size_t gnsize = sizeof(geoms_names) / sizeof(geoms_names[0]);
	for( int i =0; i < gnsize; ++i )
	{
		auto marking = findFirstNode(this, geoms_names[i]  ,findNodeVisitor::not_exact);
		if(marking)
		{
			Utils::FindGeometry findVis; 
			marking->accept(findVis);

			osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*findVis._geom->asGeometry(), osg::CopyOp::DEEP_COPY_ALL
				& ~osg::CopyOp::DEEP_COPY_CALLBACKS
				);

			_decal_map->AddGeometry(geometry);

			marking->setNodeMask(0);
		}
	}


}