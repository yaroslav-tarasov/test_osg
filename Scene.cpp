#include "stdafx.h"

#include "high_res_timer.h"
#include "bi/BulletInterface.h"
#include "bi/RigidUpdater.h"


#include "Scene.h"
#include "creators.h"
#include "Ephemeris.h"
#include "sm/ShadowedScene.h"
#include "sm/ShadowMap.h"
#include "sm/ViewDependentShadowMap.h"
#include "Terrain.h"

#include "find_node_visitor.h" 

#include "pickhandler.h"

using namespace avScene;

std::vector<osg::ref_ptr<osg::Node>> _lamps;

namespace {

    inline osg::Vec3f lights_offset(std::string name)
    {
        if (name == "sheremetyevo")
            return osg::Vec3f(18, 17, .2f);
        else if (name == "adler")
            return osg::Vec3f(0 , 0 , .2f);

        return osg::Vec3f();
    }

    struct value_getter
    {
        value_getter(std::string const& line)
        {
            boost::split(values_, line, boost::is_any_of(" \t"), boost::token_compress_on);
        }

        template <class T>
        T get(size_t index)
        {
            return boost::lexical_cast<T>(values_[index]);
        }

    private:
        std::vector<std::string> values_;
    };

template<typename S> 
inline osg::Vec3f polar_point_2(S range, S course )
{
    return osg::Vec3( (S)range * sin(cg::grad2rad(course)),
        (S)range * cos(cg::grad2rad(course)),0);
}

typedef osg::ref_ptr<osg::Group> navaid_group_node_ptr;

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

void fill_navids(std::string file, std::vector<osg::ref_ptr<osg::Node>>& cur_lamps, osg::Group* parent, osg::Vec3f const& offset)
{
    //     if (!boost::filesystem::is_regular_file(file))
    //         LogWarn("No lights for airport found: " << file.string());


    //osg::ref_ptr<osg::Geode> red_light   = CreateLight(creators::red_color,std::string("red"),nullptr);
    //osg::ref_ptr<osg::Geode> blue_light  = CreateLight(creators::blue_color,std::string("blue"),nullptr);
    //osg::ref_ptr<osg::Geode> green_light = CreateLight(creators::green_color,std::string("green"),nullptr);
    // osg::ref_ptr<osg::Geode> white_light = CreateLight(creators::white_color,std::string("white_blink"),new effects::BlinkNode(white_color,gray_color));


    std::ifstream ifs(file);

    for (auto it = cur_lamps.begin(); it != cur_lamps.end(); ++it)
        parent->removeChild(it->get());
    cur_lamps.clear();

    navaid_group_node_ptr navid_node_ptr = nullptr;

    bool group_ready = false;

    while (ifs.good())
    {
        char buf[0x400] = {};
        ifs.getline(buf, 0x400);

        std::string line = buf;
        boost::trim(line);

        // skip comments

        if (line.size()==0) continue;

        if (boost::starts_with(line, "//"))
        {
            boost::erase_all(line," ");
            if (boost::starts_with(line, "//#"))
            {
                boost::trim_if(line, boost::is_any_of("/#"));
                //navid_node_ptr.reset(static_cast<victory::navaid_group_node *>(fabric->create(victory::node::NT_NavAidGroup).get()));
                navid_node_ptr.release();
                navid_node_ptr = new osg::Group();
                navid_node_ptr->setName(line);
                group_ready = true;
            }
            else
                continue;
        }

        value_getter items(line);

        if (items.get<std::string>(0) == "FireLine:" || items.get<std::string>(0) == "FireLineHa:")
        {
            size_t ofs = items.get<std::string>(0) == "FireLineHa:" ? 3 : 0;

            osg::Vec3f pos (items.get<float>(ofs + 2), items.get<float>(ofs + 4), items.get<float>(ofs + 3));
            osg::Vec3f dir (polar_point_2(1.f, items.get<float>(ofs + 5)));
            float      len   = items.get<float>(ofs + 6);
            float      step  = items.get<float>(ofs + 7);
            osg::Vec4f clr (items.get<float>(ofs + 8), items.get<float>(ofs + 9), items.get<float>(ofs + 10),1.0f); 

            // special AI, hello from VNIIRA
            size_t count = 0;
            if (!cg::eq_zero(step) && !cg::eq_zero(len))
            {
                count = size_t(cg::ceil(len / step));

                if (step > 2)
                    ++count;
            }

            for (size_t i = 0; i < count; ++i)
            {
                osg::Vec3f p = pos + dir * step * i  + offset;

                //victory::navaid_group_node::LightData lamp = {p, clr, .1,40000,/*.01f, 4000.f,*/ cg::range_2f(), cg::range_2f(), 1, 0, 0};
                //navid_group->AddLight(lamp);
                osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
             
                pat->addChild(CreateLight(clr,std::string("light"),nullptr));
                pat->setPosition(osg::Vec3f(p.x(),p.y(),p.z()));
                navid_node_ptr->addChild(pat);

                const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED| osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF;
                osg::StateSet* ss = pat->getOrCreateStateSet();
                ss->setAttribute(new osg::Program(),value);
                ss->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
                ss->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
                ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE );
                ss->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");


            }
        }

        if(navid_node_ptr && group_ready)
        {
            cur_lamps.push_back(navid_node_ptr);
            parent->addChild(navid_node_ptr);
            group_ready = false;
        }

    }


    //cur_lamps.push_back(navid_group);
    //parent->add(navid_group);
}

}


osg::ref_ptr<Scene>	 Scene::_scenePtr;

//////////////////////////////////////////////////////////////////////////
bool Scene::Create( osg::ArgumentParser& cArgs, osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr )
{
    _scenePtr = new Scene();
    if ( _scenePtr->Initialize( cArgs, cTraitsPtr, cTraitsPtr.valid() ? cTraitsPtr->width : 0, cTraitsPtr.valid() ? cTraitsPtr->height : 0 ) == false )
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

        // Release scene
        _scenePtr = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
Scene* Scene::GetInstance()
{
    return _scenePtr.get();
}



//////////////////////////////////////////////////////////////////////////
Scene::Scene()
{
    // Common nodes for scene etc.
    _commonNode = new Group();
    osg::StateSet * pCommonStateSet = _commonNode->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");
    addChild(_commonNode.get());

    // Add backface culling to the whole bunch
    //osg::StateSet * pSS = getOrCreateStateSet();
    //pSS->setNestRenderBins(false);
    //pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    //// disable alpha writes for whole bunch
    //pSS->setAttribute(new osg::ColorMask(true, true, true, false)); 

}

//////////////////////////////////////////////////////////////////////////
Scene::~Scene()
{
}

//////////////////////////////////////////////////////////////////////////
bool Scene::Initialize( osg::ArgumentParser& cArgs, osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr, int nWidth, int nHeight)
{
    osg::StateSet* pGlobalStateSet = getOrCreateStateSet();
    osg::StateSet* pCommonStateSet = getCommonNode()->getOrCreateStateSet();
    
    const int nAntialiasing = 8;

    if ( cTraitsPtr.valid())
    {
        cTraitsPtr->samples = nAntialiasing;
    }
    else 
    {
        osg::DisplaySettings::instance()->setNumMultiSamples( nAntialiasing );
    }

    // Create viewer and 
    _viewerPtr = new osgViewer::Viewer(cArgs);
    
    _viewerPtr->apply(new osgViewer::SingleScreen(1));


    // Set up camera
    if ( cTraitsPtr.valid() == true )
    {
        osg::ref_ptr<osg::GraphicsContext> cGraphicsContextPtr = osg::GraphicsContext::createGraphicsContext( cTraitsPtr.get() );

        _viewerPtr->getCamera()->setGraphicsContext(cGraphicsContextPtr.get());
        _viewerPtr->getCamera()->setViewport(new osg::Viewport(0, 0, nWidth, nHeight));
    } 
    else if (getenv("OSG_SCREEN") == NULL)
    {
        //_viewerPtr->setUpViewInWindow(20, 20, 820, 620);
    }

    _viewerPtr->getCamera()->setClearColor(osg::Vec4(0.f, 0.f, 0.f, 1.f));
    //FIXME TODO //setProjectionMatrixFromConfig();

    _viewerPtr->getCamera()->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);    
    // _viewerPtr->getCamera()->setSmallFeatureCullingPixelSize(10.0F);

    _viewerPtr->setSceneData( this );
    //_viewerPtr->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // TODO: enabled this for instructor tab, need implement special setting
    //_viewerPtr->setReleaseContextAtEndOfFrameHint(false); 

    // disable ESC key
    // _viewerPtr->setKeyEventSetsDone(0);

    //
    // Set up the camera manipulators.
    //
   
    // Use a default camera manipulator
    osgGA::FirstPersonManipulator* manip = new osgGA::FirstPersonManipulator;
     //new osgGA::DriveManipulator()
     //new osgGA::TerrainManipulator() 
     //new osgGA::OrbitManipulator() 
     //new osgGA::FirstPersonManipulator() 
     //new osgGA::SphericalManipulator() 

    manip->setAcceleration(0);
    manip->setMaxVelocity(1);
    manip->setWheelMovement(10,true);
    //manip->setWheelMovement(0.001,true);
    _viewerPtr->setCameraManipulator(manip);
    //manip->setHomePosition(osg::Vec3(470,950,100), osg::Vec3(0,0,100), osg::Z_AXIS);
    //manip->home(0);

    //
    // Add event handlers to the viewer
    //
    _viewerPtr->addEventHandler(new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()));
    _viewerPtr->addEventHandler(new osgViewer::StatsHandler);
    _viewerPtr->addEventHandler(new PickHandler());
    _viewerPtr->realize();


    //
    // Initialize particle engine
    // 

    spark::init();
    
    //
    // And some fire
    //

    spark::spark_pair_t sp =   spark::create(spark::FIRE);
    spark::spark_pair_t sp2 =  spark::create(spark::EXPLOSION);

    osg::MatrixTransform* posed = new osg::MatrixTransform(osg::Matrix::translate(osg::Vec3(400.0,400.0,50.0)));
    posed->addChild(sp.first);
    posed->addChild(sp2.first);
    _viewerPtr->addEventHandler(sp.second);
    addChild(posed);

    //
    // Create terrain / shadowed scene
    //
    createTerrainRoot();
    
    std::string scene_name("empty"); // "adler" ,"sheremetyevo"

    _terrainNode =  new avTerrain::Terrain (_terrainRoot);
    _terrainNode->create(scene_name);

    osg::Node* ct =  findFirstNode(_terrainNode,"camera_tower");

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
    


    //
    // Init physic updater
    //

    createObjects();


    //
    // Create ephemeris
    //                                                                       
    _ephemerisNode = new avSky::Ephemeris( this,
                                           _terrainNode.get(),
                                          [=](float illum){ _st->setNightMode(illum < .35);  } //  FIXME magic night value    
    );  

    //
    //  Get or create sunlight
    //
    
    if( _ephemerisNode->getSunLightSource())
    {
        _ls =_ephemerisNode->getSunLightSource();  
        _terrainRoot->addChild(_ls.get());
    }
    else
    {
        _ls = new osg::LightSource;
        _ls->getLight()->setLightNum(0);
        _terrainRoot->addChild(_ls.get());
        _ephemerisNode->setSunLightSource(_ls);
    }


    addChild( _ephemerisNode.get() );

    _viewerPtr->addEventHandler(_ephemerisNode->getEventHandler());
    
    //
    // Create weather
    //
    _weatherNode =  new osg::Group;
    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;		
    precipitationEffect->snow(0.0);
    precipitationEffect->setWind(osg::Vec3(0.2f,0.2f,0.2f));

    _weatherNode->addChild( precipitationEffect.get() );
    addChild( _weatherNode.get() );

#if 1
    fill_navids(
        scene_name + ".txt", 
        _lamps, 
        this, 
        lights_offset(scene_name) ); 
#endif

    return true;
}


void  Scene::createTerrainRoot()
{

#if defined(TEST_SHADOWS_FROM_OSG)

    const int fbo_tex_size = 1024*4;

    _st = new avShadow::ViewDependentShadowMap;

     _terrainRoot
        = new avShadow::ShadowedScene(_st.get());  

    avShadow::ShadowSettings* settings = dynamic_cast<avShadow::ShadowedScene*>(_terrainRoot.get())->getShadowSettings();

    settings->setShadowMapProjectionHint(avShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);   //ORTHOGRAPHIC_SHADOW_MAP
    settings->setBaseShadowTextureUnit(5);
    settings->setMinimumShadowMapNearFarRatio(.5);
    //settings->setNumShadowMapsPerLight(/*numShadowMaps*/2);
    //settings->setMultipleShadowMapHint(avShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setMultipleShadowMapHint(avShadow::ShadowSettings::CASCADED);
    settings->setTextureSize(osg::Vec2s(fbo_tex_size,fbo_tex_size));
    //settings->setLightNum(2);
    settings->setMaximumShadowMapDistance(1500);
    settings->setShaderHint(avShadow::ShadowSettings::NO_SHADERS);

#else
    _terrainRoot = new osg::Group;
#endif     

     addChild(_terrainRoot);
}

void Scene::createObjects()
{

    _rigidUpdater = new bi::RigidUpdater( _terrainRoot->asGroup() 
        ,[&](osg::MatrixTransform* mt){ 
            if(!findFirstNode(mt,"fire"))
            {
                spark::spark_pair_t sp3 =  spark::create(spark::FIRE,mt);
                sp3.first->setName("fire");
                mt->addChild(sp3.first);
            }
    }
    );


    //auto heli = creators::applyBM(creators::loadHelicopter(),"mi_8",true);
    //_terrainRoot->addChild(heli);


    if(_rigidUpdater.valid())
        _rigidUpdater->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );


	auto obj = creators::applyBM(creators::createObject("a_319"),"a_319",true);

	if(_rigidUpdater.valid())
		_rigidUpdater->addPhysicsAirplane( obj,
		osg::Vec3(0,0,0), osg::Vec3(0,60,0), 800.0f );


	if(_rigidUpdater.valid())
		_rigidUpdater->addUFO( obj,
		osg::Vec3(100,100,0), osg::Vec3(0,0,0), 165.0f );

	if(_rigidUpdater.valid())
		_rigidUpdater->addUFO2( obj,
		osg::Vec3(-100,-100,0), osg::Vec3(0,100000,0), 1650.0f );   // force 

    if(_rigidUpdater.valid())
        _rigidUpdater->addUFO2( obj,
        osg::Vec3(150,-150,00), osg::Vec3(0,30000,0), 1650.0f );    // force

	const bool add_planes = false;

    if (add_planes)
    {
        const std::string name = "a_319";
        osg::Node* p_copy = creators::applyBM(creators::loadAirplane(name),name,true);
        // auto p_copy = creators::loadAirplane(name); // А если без BM еще кадров 15-20 ??? Чет не вижу
        
        float rot_angle = -90.f;
        if(dynamic_cast<osg::LOD*>(p_copy))
            rot_angle = 0;  

        for ( unsigned int i=0; i<10; ++i )
        {
            for ( unsigned int j=0; j<10; ++j )
            {
                if(_rigidUpdater.valid())
                    _rigidUpdater->addPhysicsBox( new osg::Box(osg::Vec3(), 0.99f),
                    osg::Vec3((float)i, 0.0f, (float)j+0.5f), osg::Vec3(), 1.0f );
            }
        }

        const unsigned inst_num = 24;
        for (unsigned i = 0; i < inst_num; ++i)
        {
            float const angle = 2.0f * /*cg::pif*/osg::PI * i / inst_num, radius = 400.f;
            osg::Vec3 pos(radius * sin (angle), radius * cos(angle), 0.f);

            const osg::Quat quat(osg::inDegrees(rot_angle), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(180.f * (i & 1)) - angle  , osg::Z_AXIS ); 


            osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(pos));
            //positioned->setDataVariance(osg::Object::STATIC);

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
            //rotated->setDataVariance(osg::Object::STATIC);

            positioned->addChild(rotated);
            //rotated->addChild(p_copy);

            if(_rigidUpdater.valid())
             _rigidUpdater->addPhysicsAirplane( p_copy,
                pos, osg::Vec3(0,0,0), 800.0f );

            osg::Vec3 pos2( radius * sin (angle),   radius * cos(angle), 0.f);

            if(_rigidUpdater.valid())
                _rigidUpdater->addPhysicsAirplane( p_copy,
                pos2, osg::Vec3(0,60,0), 1000.0f );

            // add it
            // _terrainRoot->addChild(positioned);  

            if (i==1) 
            {    
                auto manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( p_copy->getUpdateCallback() );
                if ( manager_ )
                {   

                    const osgAnimation::AnimationList& animations =
                        manager_->getAnimationList();

                    for ( unsigned int i=0; i<animations.size(); ++i )
                    {
                        const std::string& name = animations[i]-> getName();
                        if ( name==std::string("Default") )
                        {
                            auto anim = (osg::clone(animations[i].get(), "Animation_clone", osg::CopyOp::DEEP_COPY_ALL)); 
                            // manager->unregisterAnimation(animations[i].get());
                            // manager->registerAnimation  (anim/*.get()*/);

                            animations[i]->setPlayMode(osgAnimation::Animation::ONCE);                   
                            manager_->playAnimation( /*anim*/ animations[i].get(),2,2.0 );

                        }

                    }
                }


            }

            // FIXME при копировании начинаем падать кадров на 10
            p_copy = osg::clone(p_copy, osg::CopyOp::DEEP_COPY_ALL 
                & ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
                & ~osg::CopyOp::DEEP_COPY_ARRAYS
                & ~osg::CopyOp::DEEP_COPY_IMAGES
                & ~osg::CopyOp::DEEP_COPY_TEXTURES
                & ~osg::CopyOp::DEEP_COPY_STATESETS  
                & ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
                & ~osg::CopyOp::DEEP_COPY_UNIFORMS
                & ~osg::CopyOp::DEEP_COPY_DRAWABLES
                );

            // p_copy = osg::clone(p_copy,(const osg::CopyOp) (osg::CopyOp::DEEP_COPY_CALLBACKS & osg::CopyOp::DEEP_COPY_DRAWABLES) );
            
        }
    }

    if(_rigidUpdater.valid())
        _viewerPtr->addEventHandler( _rigidUpdater/*avTerrain::bi::getUpdater().get()*/ );

}