#include "stdafx.h"
#include "Terrain.h"

#include "find_node_visitor.h" 
#include "find_tex_visitor.h"
#include "find_animation.h"

#include "creators.h"
#include <windows.h>
#include "shaders.h"
#include "ct_visitor.h"

#include "materials_visitor.h"
#include "pugixml.hpp"
#include "shadow_map.h"

#include "sm/ShadowedScene.h"
#include "sm/ShadowMap.h"
#include "sm/ViewDependentShadowMap.h"

#include "high_res_timer.h"
#include "bi/BulletInterface.h"

namespace avTerrain
{

namespace bi
{
    class RigidUpdater : public osgGA::GUIEventHandler
    {
    public:
        typedef std::function<void(osg::MatrixTransform* mt)> on_collision_f;
    public:
        RigidUpdater( osg::Group* root, on_collision_f on_collision = nullptr ) 
            : _root        (root)
            , _on_collision(on_collision) 
        {}

        void addGround( const osg::Vec3& gravity )
        {
            BulletInterface::instance()->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), gravity,
                [&](int id){
                    if(_on_collision)
                        _on_collision(_physicsNodes[id].get());   
            } );
        }

        void addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();

            osg::ComputeBoundsVisitor cbv;
            node->accept( cbv );
            const osg::BoundingBox& bb = cbv.getBoundingBox();

            float xm = bb.xMax() - bb.xMin();
            float ym = bb.yMax() - bb.yMin();
            float zm = bb.zMax() - bb.zMin();

            osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(osg::Vec3(0.0,-(bb.yMax() - bb.yMin())/2.0f,-(bb.yMax() - bb.yMin())/2.0f)));

            const osg::Quat quat(osg::inDegrees(-90.f), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(0.f)   , osg::Z_AXIS ); 

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));

            rotated->addChild(node);
            positioned->addChild(rotated);


            osg::Vec3 half_length ( (bb.xMax() - bb.xMin())/2.0f,(bb.zMax() - bb.zMin())/2.0f,(bb.yMax() - bb.yMin()) /2.0f );
            BulletInterface::instance()->createBox( id, half_length, mass );
            addPhysicsData( id, positioned, pos, vel, mass );
        }

        void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();
            BulletInterface::instance()->createBox( id, shape->getHalfLengths(), mass );
            addPhysicsData( id, shape, pos, vel, mass );
        }

        void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
        {
            int id = _physicsNodes.size();
            BulletInterface::instance()->createSphere( id, shape->getRadius(), mass );
            addPhysicsData( id, shape, pos, vel, mass );
        }

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
        {
            osgViewer::View* view = static_cast<osgViewer::View*>( &aa );
            if ( !view || !_root ) return false;

            switch ( ea.getEventType() )
            {
            case osgGA::GUIEventAdapter::KEYUP:
                if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Return )
                {
                    osg::Vec3 eye, center, up, dir;
                    view->getCamera()->getViewMatrixAsLookAt( eye, center, up );
                    dir = center - eye; dir.normalize();
                    addPhysicsSphere( new osg::Sphere(osg::Vec3(), 0.5f), eye, dir * 200.0f, 2.0 );
                }
                break;
            case osgGA::GUIEventAdapter::FRAME:
                {
                    double dt = _hr_timer.get_delta();
                    BulletInterface::instance()->simulate( /*0.02*/dt );
                    for ( NodeMap::iterator itr=_physicsNodes.begin();
                        itr!=_physicsNodes.end(); ++itr )
                    {
                        osg::Matrix matrix = BulletInterface::instance()->getMatrix(itr->first);
                        itr->second->setMatrix( matrix );
                    }
                }
                break;
            default: break;
            }
            return false;
        }

    protected:
        void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass )
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable( new osg::ShapeDrawable(shape) );

            osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
            mt->addChild( geode.get() );
            _root->addChild( mt.get() );

            BulletInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
            BulletInterface::instance()->setVelocity( id, vel );
            _physicsNodes[id] = mt;
        }

        void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
            const osg::Vec3& vel, double mass )
        {
            osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
            mt->addChild( node/*.get()*/ );
            _root->addChild( mt.get() );

            BulletInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
            BulletInterface::instance()->setVelocity( id, vel );
            _physicsNodes[id] = mt;
        }
    private:
        typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
        NodeMap _physicsNodes;
        osg::observer_ptr<osg::Group> _root;
        high_res_timer                _hr_timer;
        on_collision_f                _on_collision;
    };

    osg::ref_ptr<osgGA::GUIEventHandler>& getUpdater()
    {
        static osg::ref_ptr<osgGA::GUIEventHandler> updater;
        return  updater;
    }


}


osg::Node* createModel( osg::ref_ptr<osg::LightSource>& ls)
{
    osg::Vec3 center(0.0f,0.0f,300.0f);
    float radius = 600.0f;
    high_res_timer                _hr_timer;

#if defined(TEST_SHADOWS) || defined(TEST_SHADOWS_FROM_OSG)

    const int fbo_tex_size = 1024*8;

    osg::ref_ptr<testShadow::ViewDependentShadowMap> st = new testShadow::ViewDependentShadowMap;

    // st->setNightMode(true);

    osg::ref_ptr<testShadow::ShadowedScene> root
        = new testShadow::ShadowedScene(st.get());  

    testShadow::ShadowSettings* settings = root->getShadowSettings();

    settings->setShadowMapProjectionHint(testShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);   //ORTHOGRAPHIC_SHADOW_MAP
    settings->setBaseShadowTextureUnit(5);
    settings->setMinimumShadowMapNearFarRatio(.5);
    //settings->setNumShadowMapsPerLight(/*numShadowMaps*/2);
    //settings->setMultipleShadowMapHint(testShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setMultipleShadowMapHint(testShadow::ShadowSettings::CASCADED);
    settings->setTextureSize(osg::Vec2s(fbo_tex_size,fbo_tex_size));
    //settings->setLightNum(2);
    settings->setMaximumShadowMapDistance(2000);
    settings->setShaderHint(testShadow::ShadowSettings::NO_SHADERS);

    osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
    source->getLight()->setPosition(osg::Vec4(0, 0, 20, 0));
    source->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    source->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    // Scene
    //st->setLight(source->getLight());
    source->getLight()->setLightNum(2);
    // ssm = st;
    ls = source;

    root->addChild(source.get());


#else
    osg::Group* root = new osg::Group;
#endif

    float baseHeight = 0.0f; //center.z();//-radius*0.5;
#ifdef TEST_REAL_SCENE 
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 

#if 1
    const char* scene_name = "sheremetyevo.open.osgb";//"sheremetyevo.lod0.osgb";//"sheremetyevo.lod0.dae"; //"adler.open.dae";// "sheremetyevo.open.dae"; //"adler.open.dae"  
    const char* mat_file_name = "sheremetyevo.open.dae"; //scene_name;//
#else
    const char* scene_name = "adler.osgb";//"sheremetyevo.lod0.dae"; //"adler.open.dae";// "sheremetyevo.open.dae"; //"adler.open.dae"  
    const char* mat_file_name = "adler.open.dae"; //scene_name;//
#endif


    osg::Node* scene = osgDB::readNodeFile(scene_name);  // "adler.osgb"

    scene->setName("scene");

    findNodeVisitor findLod3("lod3"); 
    scene->accept(findLod3);
    auto lod3 =  findLod3.getFirst();

    if(lod3) 
        lod3->setNodeMask(0); // Убираем нафиг Lod3 

    osg::MatrixTransform* baseModel = new osg::MatrixTransform;
    baseModel->setMatrix(osg::Matrix::rotate(quat0));
    baseModel->addChild(scene);

    MaterialVisitor::namesList nl;
    nl.push_back("building");
    nl.push_back("tree");
    // Для теней
    nl.push_back("ground"); 
    nl.push_back("concrete");
    nl.push_back("mountain");

    nl.push_back("sea");
    nl.push_back("railing");
    nl.push_back("panorama");
    MaterialVisitor mv ( nl, createMaterial,computeAttributes,mat::reader::read(mat_file_name));
    scene->accept(mv);

    // All solid objects
    osg::StateSet * pCommonStateSet = scene->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");
    pCommonStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    pCommonStateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

    // Scene 
    // Add backface culling to the whole bunch
    //osg::StateSet * pSS = adler->getOrCreateStateSet();
    //pSS->setNestRenderBins(false);
    //pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    // disable alpha writes for whole bunch
    //pSS->setAttribute(new osg::ColorMask(true, true, true, false));


#else
    osg::Node* baseModel = creators::createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius*3);
#endif


    OSG_WARN << "Время загрузки сцены: " << _hr_timer.get_delta() << "\n";

    auto ret_array  = creators::createMovingModel(center,radius*0.8f);

    osg::Node* movingModel = ret_array[0];

    auto heli = creators::applyBM(creators::loadHelicopter(),"mi_8",true);
    root->addChild(heli);

    const bool add_planes = true;

    if (add_planes)
    {
        auto p_copy = creators::applyBM(creators::loadAirplane(),"a_319",true);

        bi::RigidUpdater* rigidUpdater = new bi::RigidUpdater( root.get() 
            ,[&](osg::MatrixTransform* mt){ 
                findNodeVisitor findFire("fire"); 
                mt->accept(findFire);
                if(!findFire.getFirst())
                {
                    spark::spark_pair_t sp3 =  spark::create(spark::FIRE,mt);
                    sp3.first->setName("fire");
                    mt->addChild(sp3.first);
                }
        }
        );

        rigidUpdater->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );
        for ( unsigned int i=0; i<10; ++i )
        {
            for ( unsigned int j=0; j<10; ++j )
            {
                rigidUpdater->addPhysicsBox( new osg::Box(osg::Vec3(), 0.99f),
                    osg::Vec3((float)i, 0.0f, (float)j+0.5f), osg::Vec3(), 1.0f );
            }
        }

        bi::getUpdater() = rigidUpdater;

        const unsigned inst_num = 24;
        for (unsigned i = 0; i < inst_num; ++i)
        {
            float const angle = 2.0f * /*cg::pif*/osg::PI * i / inst_num, radius = 200.f;
            osg::Vec3 pos(radius * sin (angle), radius * cos(angle), 0.f);

            const osg::Quat quat(osg::inDegrees(-90.f), osg::X_AXIS,                      
                osg::inDegrees(0.f) , osg::Y_AXIS,
                osg::inDegrees(180.f * (i & 1)) - angle  , osg::Z_AXIS ); 


            osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(pos));
            //positioned->setDataVariance(osg::Object::STATIC);

            osg::MatrixTransform* rotated = new osg::MatrixTransform(osg::Matrix::rotate(quat));
            //rotated->setDataVariance(osg::Object::STATIC);

            positioned->addChild(rotated);
            //rotated->addChild(p_copy);

            rigidUpdater->addPhysicsAirplane( p_copy,
                pos, osg::Vec3(0,0,0), 800.0f );

            osg::Vec3 pos2( radius * sin (angle),   radius * cos(angle), 0.f);

            rigidUpdater->addPhysicsAirplane( p_copy,
                pos2, osg::Vec3(0,60,0), 1000.0f );

            // add it
            root->addChild(positioned);

            p_copy = osg::clone(p_copy, osg::CopyOp::DEEP_COPY_ALL 
                & ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
                & ~osg::CopyOp::DEEP_COPY_ARRAYS
                & ~osg::CopyOp::DEEP_COPY_IMAGES
                & ~osg::CopyOp::DEEP_COPY_TEXTURES
                );
        }
    }

    root->addChild(baseModel);


    baseModel->setName("baseModel");
    movingModel->setName("movingModel");

    root->addChild(movingModel);

    OSG_WARN << "Время загрузки копирования самолетов: " << _hr_timer.get_delta() << "\n";

    // osgDB::writeNodeFile(*movingModel,"test_osg_struct.osgt");

    return root.release();
}


}