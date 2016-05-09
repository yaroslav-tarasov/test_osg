#include "stdafx.h"

#include "av/precompiled.h"

#include <osgUtil/Tessellator>

#include "visitors/find_tex_visitor.h"
#include "visitors/find_animation.h"
#include "visitors/ct_visitor.h"
#include "visitors/materials_visitor.h"

#include "utils/high_res_timer.h"
#include "utils/callbacks.h"
#include "utils/materials.h"

#include "av/avScene/Scene.h"
#include "av/avLights/LightManager.h"
#include "av/avLights/NavAid.h"

#include "av/shaders.h"
#include "Terrain.h"
#include "Grass.h"
#include "Grass2.h"



#include "utils/empty_scene.h"
#include "utils/async_load.h"

namespace
{
    //typedef osg::ref_ptr<osgSim::LightPointNode> navaid_group_node_ptr;
	typedef osg::ref_ptr<avScene::NavAidGroup> navaid_group_node_ptr;

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
        else if (name == "eisk")
            return osg::Vec3f(0 , 0 , 0.5f);
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
        else if (name == "eisk")
            return "eisk.scn";
        else if (name == "vnukovo")
            return "vnukovo.scn";
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

        std::ifstream ifs(file);

        for (auto it = cur_lamps.begin(); it != cur_lamps.end(); ++it)
            parent->removeChild(it->get());
        cur_lamps.clear();

        navaid_group_node_ptr navid_node = nullptr;

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
                    navid_node.release();
                    navid_node = new avScene::NavAidGroup;
                    navid_node->setName(line);
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

					avScene::LightManager::Light data;
					data.transform  = nullptr;  
					data.spotFalloff = cg::range_2f();
					data.distanceFalloff = cg::range_2f(0.9f, 15.0f);
					data.position =  from_osg_vector3(p);
     				const float heading = osg::DegreesToRadians(0.f);
					const float pitch   = osg::DegreesToRadians(0.f);
				    data.direction = cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

					data.active = true;
					data.lm_only = true;

					navid_node->addLight(pnt,data);
                    //navid_node->addLightPoint(pnt);

                }
            }

            if(navid_node && group_ready)
            {
                cur_lamps.push_back(navid_node);
                parent->addChild(navid_node);
                group_ready = false;
            }

        }


        //cur_lamps.push_back(navid_group);
        //parent->add(navid_group);
    }



}

namespace avTerrain
{
    const osg::Quat quat0(osg::inDegrees(-90.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(0.f)  , osg::Z_AXIS ); 

Terrain::Terrain (osg::Group* sceneRoot)
    : _sceneRoot (sceneRoot)
    , _lightsHandler(avScene::/*GlobalInfluence*/LocalInfluence)
{
    setNodeMask( PICK_NODE_MASK | REFLECTION_MASK/*| cReceivesShadowTraversalMask*/ );


    //
    // Callbacks
    //

    // callbacks setup
    setCullCallback(Utils::makeNodeCallback(this, &Terrain::cull, true));

}

void  Terrain::Create( const std::string& cFileName )
{
    Database::fpl_wrap  fpl(cFileName);

    auto wf =  [this](std::string cFileName)->osg::Node* {
 
    const   osg::Vec3 center(0.0f,0.0f,300.0f);
    const   float radius = 600.0f;
    high_res_timer                _hr_timer;
    osg::ref_ptr<osg::PositionAttitudeTransform>   baseModel = new osg::PositionAttitudeTransform;


    float baseHeight = 0.0f; 
    
    high_res_timer hr_timer;

    if(cFileName != "empty" && !cFileName.empty() )
    {

    std::string scene_name;
    std::string mat_file_name;

    if(cFileName == "sheremetyevo")
    {
        scene_name =    "sheremetyevo.osgb";//"sheremetyevo.open.osgb";//"sheremetyevo.osgb"; 
        mat_file_name = "sheremetyevo.dae.mat.xml"; //"sheremetyevo.open.dae.mat.xml"; 
    }
    else if(cFileName == "adler")
    {
        scene_name = "adler.osgb";  
        mat_file_name = "adler.open.dae.mat.xml"; 
    }
	else if(cFileName == "minsk")
	{
        scene_name = "minsk.dae";  
        mat_file_name = "minsk.dae.mat.xml"; 
	}
    else if(cFileName == "lipetsk")
    {
        scene_name = "lipetsk.dae";  
        mat_file_name = "lipetsk.dae.mat.xml"; 
    }
    else if(cFileName == "eisk")
    {
        scene_name = "eisk.dae";  
        mat_file_name = "eisk.dae.mat.xml"; 
    } 
    else if(cFileName == "vnukovo")
    {
        scene_name = "vnukovo.dae";  
        mat_file_name = "vnukovo.dae.mat.xml"; 
    }

    osg::Node* scene = osgDB::readNodeFile(cFileName + "/"+ scene_name);  
    
    if (scene->asTransform())
        baseModel = scene->asTransform()->asPositionAttitudeTransform();
    else
        baseModel->addChild(scene);

    scene->setName("scene");

    auto lod0 =  findFirstNode(scene,"lod0");
    auto lod3 =  findFirstNode(scene,"lod3");

#if 1  // ## TEST01
    if(lod3) 
        lod3->setNodeMask(0); 
#else
    if(lod0) 
        lod0->setNodeMask(0); 
#endif

#if 0
    baseModel = new osg::MatrixTransform;
    baseModel->setMatrix(osg::Matrix::rotate(quat0));
    baseModel->addChild(scene);
#endif


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
    nl.push_back("default");
    nl.push_back("color");

    MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,sp::_2,cFileName,sp::_3,sp::_4),creators::computeAttributes,utils::singleton<mat::reader>::instance().read(cfg().path.data + "/areas/" + cFileName + "/"+mat_file_name));
    scene->accept(mv);
    

    FIXME(Test code)
    if(cFileName == "eisk")
    {
        findNodeByType< osg::Geode> geode_finder;  
        geode_finder.apply(*scene);

        osg::Geode*    gnode = dynamic_cast<osg::Geode*>(geode_finder.getLast()); 
        osg::Geometry* mesh = gnode->getDrawable(0)->asGeometry();

        osgUtil::Tessellator tscx;
        tscx.setBoundaryOnly(true);
        tscx.setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD); // tessellation - ODD, only show boundary.
        tscx.retessellatePolygons(*mesh);
    } 



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
    
    baseModel->setAttitude(osg::Quat(osg::inDegrees(0.0),osg::X_AXIS));
    // baseModel->setPosition(osg::Vec3(0.,0.f,0.)); 
    baseModel->setDataVariance(osg::Object::STATIC);

    OSG_WARN << "Время загрузки сцены: " << _hr_timer.set_point() << "\n";

#if 0
    Grass* grass = new Grass();
    grass->setWindFactor(1.0);
    addChild(grass);
    _grass = grass;
#endif

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

    OSG_WARN << "Время загрузки копирования моделей: " << _hr_timer.set_point() << "\n";
    
    auto light_masts = findNodes(baseModel,"lightmast_",findNodeVisitor::not_exact);

    for (auto it = light_masts.begin();it != light_masts.end();++it)
    {   
        if((*it)->asTransform())
        {
            std::string node_name((*it)->getName());
            std::string mast_index = node_name.substr(node_name.find("_")+1);
			
			avScene::LightManager::Light data;
			data.transform = (*it)->asTransform()->asMatrixTransform();

			data.spotFalloff = cg::range_2f(cg::grad2rad(15.f), cg::grad2rad(45.f));
			data.distanceFalloff = cg::range_2f(80.f, 220.f);
			data.color = cg::colorf(0.99,0.99,0.99);
			data.position = cg::point_3f(0,0,0);

			const float heading = osg::DegreesToRadians(0.f);
			const float pitch = osg::DegreesToRadians(0.f/*-90.f*/);
			data.direction = cg::as_vector(cg::point_3f(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch) ));

//#ifndef ASYNC_OBJECT_LOADING
            avScene::LightManager::GetInstance()->addLight(data);
//#endif

        }
    }

    force_log fl;
    LOG_ODS_MSG( "Terrain::create  " << hr_timer.set_point() << "\n");

    return nullptr;

    };

#ifdef ASYNC_OBJECT_LOADING
    //_lnt =   new utils::LoadNodeThread ( boost::bind<osg::Node*>(wf, name) );
	wf(cFileName);
#else
    wf(cFileName);
#endif
    
    std::string scn_file_name =  osgDB::findFileInPath(lights_file(cFileName), fpl.fpl_,osgDB::CASE_INSENSITIVE);
    
    if(!scn_file_name.empty())
        fill_navids(
            scn_file_name, 
            avScene::Scene::GetInstance()->getLamps(),
            _sceneRoot, 
            lights_offset(cFileName) ); 

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
