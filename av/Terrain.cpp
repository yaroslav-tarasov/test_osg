#include "stdafx.h"

#include "Scene.h"
#include "Terrain.h"
#include "LightManager.h"

#include "visitors/find_tex_visitor.h"
#include "visitors/find_animation.h"
#include "visitors/ct_visitor.h"
#include "visitors/materials_visitor.h"

#include "shaders.h"

#include "utils/high_res_timer.h"
#include "utils/callbacks.h"
#include "utils/materials.h"

//#include <osgwTools/AbsoluteModelTransform.h>
//#include "phys/BulletInterface.h"
//#include "phys/BulletMTInterface.h"
//#include "phys/RigidMTUpdater.h"


#include "av/Grass.h"
#include "av/Grass2.h"

#include "utils/empty_scene.h"

namespace
{
    typedef osg::ref_ptr<osgSim::LightPointNode> navaid_group_node_ptr;

    template<typename S> 
    inline osg::Vec3f polar_point_2(S range, S course )
    {
        return osg::Vec3( (S)range * sin(cg::grad2rad(course)),
            (S)range * cos(cg::grad2rad(course)),0);
    }

    inline osg::Vec3f lights_offset(std::string name)
    {
        if (name == "sheremetyevo")
            return osg::Vec3f(18, 17, .2f);
        else if (name == "adler")
            return osg::Vec3f(0 , 0 , .2f);
        else if (name == "minsk")
            return osg::Vec3f(0 , 0 , .5f);

        return osg::Vec3f();
    }

    inline std::string lights_file(std::string name)
    {
        if (name == "sheremetyevo")
            return "sheremetyevo.txt";
        else if (name == "adler")
            return "sochi.scn";
        else if (name == "minsk")
            return "minsk.scn";

        return "";
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

        bool valid()
        {
            return values_.size()>0;
        }

    private:
        std::vector<std::string> values_;
    };

    void fill_navids(std::string file, std::vector<osg::ref_ptr<osg::Node>>& cur_lamps, osg::Group* parent, osg::Vec3f const& offset)
    {
        if (!boost::filesystem::is_regular_file(file))
            LogWarn("No lights for airport found: " << file);

        const bool usePointSprites = true;
        osg::Texture2D *tex;
        if(usePointSprites)
        {
            tex = new osg::Texture2D();
            tex->setImage(osgDB::readImageFile("Images/particle.rgb"));
        }



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
                    navid_node_ptr = new osgSim::LightPointNode;
                    navid_node_ptr->setName(line);
                    //
                    osg::StateSet* set = navid_node_ptr->getOrCreateStateSet();

                    if (usePointSprites)
                    {
                        navid_node_ptr->setPointSprite();

                        // Set point sprite texture in LightPointNode StateSet.

                        set->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
#if 0                    
                        osg::ref_ptr<osg::Program> cLightPointProg = creators::createProgram("simlight").program; 
                        cLightPointProg->setName("LightLayerShader");
                        set->setAttribute(cLightPointProg.get());
#endif
                    }

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

                    osgSim::LightPoint pnt;

                    pnt._position.set(p.x(),p.y(),p.z());
                    pnt._color = clr;
                    if (!usePointSprites)
                        pnt._radius = 0.3f;
                    else
                        pnt._radius = 0.6f;

                    navid_node_ptr->addLightPoint(pnt);

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

namespace details
{
    // Use a thread to call osgDB::readNodeFile.
    struct  LoadNodeThread : public OpenThreads::Thread
    {
        typedef boost::function<void ()> on_work_f  ; 

        LoadNodeThread( on_work_f work )
            : _work( work )
            , _node(nullptr)
        {
            startThread();
        }

        ~LoadNodeThread()
        {}

        void run()
        {
            if( _work )
                _work();
        }

        on_work_f                 _work;
        osg::ref_ptr< osg::Node > _node;
    };
}

namespace avTerrain
{
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 

    const osg::Quat quat1(osg::inDegrees(90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 

Terrain::Terrain (osg::Group* sceneRoot)
    : _lightsHandler(avScene::GlobalInfluence)
    , _sceneRoot (sceneRoot)
{
    setNodeMask( PICK_NODE_MASK );

    //
    // Callbacks
    //

    // callbacks setup
    setCullCallback(utils::makeNodeCallback(this, &Terrain::cull, true));

}

void  Terrain::create( const std::string& name )
{
    _sceneName = name;

    auto wf =  [this]() {

    const std::string& name = _sceneName;
    const   osg::Vec3 center(0.0f,0.0f,300.0f);
    const   float radius = 600.0f;
    high_res_timer                _hr_timer;
    osg::MatrixTransform*         baseModel = new osg::MatrixTransform;

    float baseHeight = 0.0f; 

    if(name != "empty" && !name.empty() )
    {

    std::string scene_name;
    std::string mat_file_name;

    if(name == "sheremetyevo")
    {
        scene_name =    "sheremetyevo.osgb";//"sheremetyevo.open.osgb";//"sheremetyevo.osgb"; 
        mat_file_name = "sheremetyevo.dae.mat.xml"; //"sheremetyevo.open.dae.mat.xml"; 
    }
    else if(name == "adler")
    {
        scene_name = "adler.osgb";  
        mat_file_name = "adler.open.dae.mat.xml"; 
    }
	else if(name == "minsk")
	{
        scene_name = "minsk.dae";  
        mat_file_name = "minsk.dae.mat.xml"; 
	}

 

    osg::Node* scene = osgDB::readNodeFile(name + "/"+ scene_name);  

    // Здесь был Минск
    //osg::PositionAttitudeTransform* pat = scene->asTransform()->asPositionAttitudeTransform();
    //pat->setAttitude( osg::Quat(osg::inDegrees(90.0),osg::X_AXIS,osg::inDegrees(/*-49.0 - 33.0*/0.0),osg::Y_AXIS,osg::inDegrees(0.0),osg::Z_AXIS));

    scene->setName("scene");

    auto lod3 =  findFirstNode(scene,"lod3");

    if(lod3) 
        lod3->setNodeMask(0); 

    baseModel = new osg::MatrixTransform;
    baseModel->setMatrix(osg::Matrix::rotate(quat0));
    baseModel->addChild(scene);

    MaterialVisitor::namesList nl;
    nl.push_back("building");
    nl.push_back("tree");
    nl.push_back("ground"); 
    nl.push_back("concrete");
    nl.push_back("mountain");
    nl.push_back("sea");
    nl.push_back("railing");
    nl.push_back("panorama");
    nl.push_back("plane");

    MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,name,sp::_2,sp::_3),creators::computeAttributes,mat::reader::read(cfg().path.data + "/areas/" + name + "/"+mat_file_name));
    scene->accept(mv);

    // All solid objects
    osg::StateSet * pCommonStateSet = scene->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(RENDER_BIN_SCENE, "RenderBin");
    pCommonStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    pCommonStateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

   }
   else
   {  
       baseModel->addChild( creators::createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius*3));
   }

    baseModel->setDataVariance(osg::Object::STATIC);

    OSG_WARN << "Время загрузки сцены: " << _hr_timer.get_delta() << "\n";

    Grass* grass = new Grass();
    grass->setWindFactor(1.0);
    addChild(grass);
    _grass = grass;
    
    //blender::Grass* grass2 = new blender::Grass();
    //addChild(grass2);

    addChild(baseModel);
    baseModel->setName("baseModel");

#if 0
    auto ret_array  = creators::createMovingModel(center,radius*0.8f);
    osg::Node* movingModel = ret_array[0];
    addChild(movingModel);
    movingModel->setName("movingModel");
#endif

    OSG_WARN << "Время загрузки копирования моделей: " << _hr_timer.get_delta() << "\n";
    
    auto light_masts = findNodes(baseModel,"lightmast_",findNodeVisitor::not_exact);

    for (auto it = light_masts.begin();it != light_masts.end();++it)
    {   
        if((*it)->asTransform())
        {
            std::string node_name((*it)->getName());
            std::string mast_index = node_name.substr(node_name.find("_")+1);
            avScene::LightManager::GetInstance()->addLight(/*boost::lexical_cast<int>(mast_index),*/(*it)->asTransform()->asMatrixTransform());
        }
    }

    };

    _lnt =   new details::LoadNodeThread ( wf );

    // osgDB::writeNodeFile(*movingModel,"test_osg_struct.osgt");

    fill_navids(
        lights_file(name), 
        /*_lamps*/avScene::Scene::GetInstance()->getLamps(), 
        _sceneRoot, 
        lights_offset(name) ); 

}

// cull method
void Terrain::cull( osg::NodeVisitor * pNV )
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
    avAssert(pCV);

    // cull down
    _lightsHandler.onCullBegin(pCV);
    pNV->traverse(*this);
    _lightsHandler.onCullEnd(pCV);
}

void Terrain::setGrassMapFactor(float val)
{
	if(_grass)
        dynamic_cast<Grass*>(_grass.get())->setGrassMapFactor(val);
}

}
