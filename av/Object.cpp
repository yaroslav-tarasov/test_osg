#include "stdafx.h"
#include "av/precompiled.h"

#include "Object.h"
#include "avCore/LOD.h"
#include "avLights/Lights.h"
#include "avLights/LightManager.h"
#include "avCore/Utils.h"
#include "avCore/XmlModel.h"

#include "materials.h"

#include "animutils.h"

using namespace avCore;

namespace creators
{

	struct fpl_wrap 
	{
		fpl_wrap(const std::string& name)
		{
			fpl_.push_back(cfg().path.data + "/models/" + name + "/");
			fpl_.push_back(cfg().path.data + "/areas/" + name + "/");
		};

		osgDB::FilePathList fpl_;
	};


namespace {
	typedef std::map< std::string, osg::ref_ptr<Object> > objectMap; 
	
	objectMap objCache;
}

// constructor and destructor
Object::Object()
{
}

Object::Object(const Object& object,const osg::CopyOp& copyop)
	: osg::Object(object,copyop)
	, _node       (copyop(object._node.get()))
	, _anim_containers (object._anim_containers)
	, _manager    (object._manager)
{

}

Object::Object(osg::Node& node)
	: _node (&node)
{
	using namespace avAnimation;
	AnimationManagerFinder finder;
	_node->accept(finder);
	_manager  = dynamic_cast<osgAnimation::BasicAnimationManager*>(finder._bm.get()); 
}

void  Object::addAnimation(const std::string& name, osg::Node* anim_container)
{
    osgAnimation::BasicAnimationManager* model = dynamic_cast<osgAnimation::BasicAnimationManager*>(anim_container->getUpdateCallback());
	
	_anim_containers.insert(make_pair(name,anim_container));
	
	for (osgAnimation::AnimationList::const_iterator it = model->getAnimationList().begin(); it != model->getAnimationList().end(); it++)
	{
        avAnimation::deleteUseless(*(*it).get());
	    
        (*it)->setName(name);
		//_animations.insert(make_pair(name,*it));
		_animations[ name ] = *it;
		_manager->registerAnimation(*it);

	}
}

void  releaseObjectCache()
{
      objCache.clear();
}

Object* createObject(std::string name, bool fclone)
{
	fpl_wrap fpl(name);
	osg::Node* object_file = nullptr;
	Object*    object      = nullptr;
	objectMap::iterator it;
    boost::optional<xml_model_t> data;

    osg::PositionAttitudeTransform* pat;

    osg::CopyOp copyop;

    if (name=="crow")
    {
       copyop =  osg::CopyOp::DEEP_COPY_ALL
           //& ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
           //& ~osg::CopyOp::DEEP_COPY_ARRAYS
           & ~osg::CopyOp::DEEP_COPY_IMAGES
           & ~osg::CopyOp::DEEP_COPY_TEXTURES    // (name != "crow"?~osg::CopyOp::DEEP_COPY_TEXTURES:osg::CopyOp::DEEP_COPY_ALL)
           & (name != "crow"?~osg::CopyOp::DEEP_COPY_STATESETS:osg::CopyOp::DEEP_COPY_ALL)
           //& ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
           //& ~osg::CopyOp::DEEP_COPY_UNIFORMS
           //& ~osg::CopyOp::DEEP_COPY_DRAWABLES
           ;
    } else
    {
        copyop =  osg::CopyOp::DEEP_COPY_ALL
            & ~osg::CopyOp::DEEP_COPY_PRIMITIVES 
            & ~osg::CopyOp::DEEP_COPY_ARRAYS
            & ~osg::CopyOp::DEEP_COPY_IMAGES
            & ~osg::CopyOp::DEEP_COPY_TEXTURES    
            & ~osg::CopyOp::DEEP_COPY_STATESETS
            & ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
            & ~osg::CopyOp::DEEP_COPY_UNIFORMS
            & ~osg::CopyOp::DEEP_COPY_DRAWABLES
            ;
    }

	if(( it = objCache.find(name))!=objCache.end() )
	{
		if(fclone)
		    object = osg::clone(it->second.get(), copyop);
		else
			object = it->second.get();

        pat = object->getNode()->asTransform()->asPositionAttitudeTransform();
	}
	else
	{
		std::string object_file_name =  osgDB::findFileInPath(name + ".xml", fpl.fpl_,osgDB::CASE_INSENSITIVE);
        std::string mat_file_name = osgDB::findFileInPath(name+".dae.mat.xml", fpl.fpl_,osgDB::CASE_INSENSITIVE);
		
		if(object_file_name.empty())
		{
			object_file_name = osgDB::findFileInPath(name+".osgb", fpl.fpl_,osgDB::CASE_INSENSITIVE);

			if(object_file_name.empty())
				object_file_name = osgDB::findFileInPath(name+".fbx", fpl.fpl_,osgDB::CASE_INSENSITIVE);

			if(object_file_name.empty())
				object_file_name = osgDB::findFileInPath(name+".dae", fpl.fpl_,osgDB::CASE_INSENSITIVE);

			if(object_file_name.empty())
				return nullptr;

			object_file = osgDB::readNodeFile(object_file_name);

		}
		else
		{
			ModelReader mr;
			data = mr.Load(object_file_name);
			object_file = osgDB::readNodeFile(osgDB::findFileInPath((*data).main_model, fpl.fpl_,osgDB::CASE_INSENSITIVE));
		}

        bool airplane = findFirstNode(object_file ,"shassi_",findNodeVisitor::not_exact)!=nullptr;
        bool vehicle  = findFirstNode(object_file ,"wheel",findNodeVisitor::not_exact)!=nullptr;
        FIXME(Палец пол и потолок при определении модели)
        bool heli     = findFirstNode(object_file ,"tailrotor",findNodeVisitor::not_exact)!=nullptr;


		osg::Node* engine = nullptr; 
		osg::Node* lod0 =  findFirstNode(object_file,"Lod0"); 
		osg::Node* lod3 =  findFirstNode(object_file,"Lod3"); 

		osg::Group* root =  dynamic_cast<osg::Group*>(findFirstNode(object_file,"Root")); 
        if(root==nullptr)
        {
            root = new osg::Group; 
            root->setName("Root");
            root->addChild( object_file ); 
            object_file = root;
        }

        root->setUserValue("id",reinterpret_cast<uint32_t>(&*root));
		// object_file->setName(name);
	
		// И какой engine мы найдем?
		//engine =  findFirstNode(object_file,"engine",findNodeVisitor::not_exact);
		//if (engine) engine_geode = engine->asGroup()->getChild(0);

		// FIXME Ну и огни в отдельный функционал
#if 0
#if 0
		auto CreateLight = [=](const osg::Vec4& fcolor,const std::string& name,osg::NodeCallback* callback)->osg::Geode* {
			osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable();
			shape1->setShape( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 0.2f) );
			osg::Geode* light = new osg::Geode;
			light->addDrawable( shape1.get() );
			dynamic_cast<osg::ShapeDrawable *>(light->getDrawable(0))->setColor( fcolor );
			light->setUpdateCallback(callback);
			light->setName(name);
			const osg::StateAttribute::GLModeValue value = osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE| osg::StateAttribute::OFF;
			light->getOrCreateStateSet()->setAttribute(new osg::Program(),value);
			light->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D(), value );
			light->getOrCreateStateSet()->setTextureAttributeAndModes( 1, new osg::Texture2D(), value );
			light->getOrCreateStateSet()->setMode( GL_LIGHTING, value );
			return light;
		};

		osg::ref_ptr<osg::Geode> red_light   = CreateLight(red_color,std::string("red"),nullptr);
		osg::ref_ptr<osg::Geode> blue_light  = CreateLight(blue_color,std::string("blue"),nullptr);
		osg::ref_ptr<osg::Geode> green_light = CreateLight(green_color,std::string("green"),nullptr);
		osg::ref_ptr<osg::Geode> white_light = CreateLight(white_color,std::string("white_blink"),new effects::BlinkNode(white_color,gray_color));

		auto addAsChild = [=](std::string root,osg::Node* child)->osg::Node* {
			auto g_point =  findFirstNode(object_file,root.c_str());
			if(g_point)  
			{
				g_point->asGroup()->addChild(child);
			}
			return g_point;
		};

		auto tail       = addAsChild("tail",white_light);
		auto strobe_r   = addAsChild("strobe_r",white_light);
		auto strobe_l   = addAsChild("strobe_l",white_light);

		auto port       = addAsChild("port",green_light);
		auto star_board = addAsChild("starboard",red_light);
#else  
        findNodeVisitor::nodeNamesList list_name;

        osgSim::LightPointNode* obj_light =  new osgSim::LightPointNode;

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
        root->accept(findNodes);
        
        findNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();
        
        auto shift_phase = cg::rand(cg::range_2(0, 255));
        
        osgSim::Sector* sector = new osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));
        
        for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
        {
             osgSim::LightPoint pnt;
             bool need_to_add = false;

             if((*it)->getName() == "tail")
             { 
                 pnt._color      = white_color;
                 need_to_add     = true;
             }

             if((*it)->getName() == "port")
             {   
                 pnt._color      = green_color;
                 need_to_add     = true;
                 pnt._sector = sector;
             }

             if((*it)->getName() == "starboard") 
             {
                 pnt._color = red_color;
                 need_to_add     = true;
                 pnt._sector = sector;
             }
            
             
             if(boost::starts_with((*it)->getName(), "strobe_")) 
             {
                 pnt._color  = white_color;
                 pnt._blinkSequence = new osgSim::BlinkSequence;
                 pnt._blinkSequence->addPulse( 0.05,
                     osg::Vec4( 1., 1., 1., 1. ) );

                 pnt._blinkSequence->addPulse( 1.5,
                     osg::Vec4( 0., 0., 0., 0. ) );

                 pnt._sector = new osgSim::AzimSector(-osg::inDegrees(170.0),-osg::inDegrees(10.0),osg::inDegrees(90.0));

                 pnt._blinkSequence->setPhaseShift(shift_phase);
                 need_to_add     = true;
             }

             pnt._position = (*it)->asTransform()->asMatrixTransform()->getMatrix().getTrans();
             pnt._radius = 0.2f;
             if(need_to_add)
                 obj_light->addLightPoint(pnt);
        }

//И чего тут делать с огнями и колбеками
FIXME( Исправить структуру под mt)
#ifndef ASYNC_OBJECT_LOADING
        if(wln_list.size()>0)
            object_file->asGroup()->addChild(obj_light);
#endif

#endif
#endif
        //
		//  А здесь будет чертов некошерный лод
		//
        osg::Node* lod_ =  findFirstNode(object_file,"lod_",findNodeVisitor::not_exact);

        if(lod_)  // Далеко не все модели имеют заданную структуру ха
        {
 		    avLod::LOD* lod = new avLod::LOD;

		    lod_->asGroup()->addChild(lod);
		    lod->addChild(lod0,0,1200);
		    lod->addChild(lod3,1200,50000);
        }
        else
        {
#if 0
            osg::Node* named_node =  findFirstNode(object_file,name,findNodeVisitor::not_exact);

            lod3 = osg::clone(named_node, osg::CopyOp::DEEP_COPY_ALL 
                & ~osg::CopyOp::DEEP_COPY_IMAGES
                & ~osg::CopyOp::DEEP_COPY_TEXTURES
                & ~osg::CopyOp::DEEP_COPY_STATESETS  
                & ~osg::CopyOp::DEEP_COPY_STATEATTRIBUTES
                & ~osg::CopyOp::DEEP_COPY_UNIFORMS
                );


            osgUtil::Simplifier simplifier;
            simplifier.setSampleRatio( 0.001 );
            lod3->accept( simplifier );
            lod3->setName("Lod3");
            
            root->addChild(lod3);
#endif     
        }

        osg::ComputeBoundsVisitor cbv;
        object_file->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();
        /*
        float xm = bb.xMax() - bb.xMin();
        float ym = bb.yMax() - bb.yMin();
        float zm = bb.zMax() - bb.zMin();*/   
        
        float xm = abs(bb.xMax()) + abs(bb.xMin());
        float ym = abs(bb.yMax()) + abs(bb.yMin());
        float zm = abs(bb.zMax()) + abs(bb.zMin());
        
        //float dx = abs(bb.xMax()) - xm / 2.f;
        //float dy = abs(bb.yMax()) - ym / 2.f;
        //float dz = abs(bb.zMax()) - zm / 2.f;
        
        float dx = -xm / 4.f; 
        float dy = -ym / 4.f; 
        float dz = -zm / 4.f; 
        
        if (object_file->asTransform())
        {
            pat = object_file->asTransform()->asPositionAttitudeTransform();
            pat->setAttitude(osg::Quat(osg::inDegrees(0.0),osg::X_AXIS));
            pat->setPosition(osg::Vec3(0,airplane?dz:0.f,0)); // FIXME Дурацкое смещение и не понятно чего с ним делать
            
        }
        else
        {
            pat = new osg::PositionAttitudeTransform; 
            pat->addChild(object_file);
            pat->setAttitude(osg::Quat(osg::inDegrees(0.0),osg::X_AXIS));
            pat->setPosition(osg::Vec3(0.,airplane?dy:0.f,0.)); // FIXME Дурацкое смещение и не понятно чего с ним делать

        }
        
        MaterialVisitor::namesList nl;
        nl.push_back("building");
        nl.push_back("default");
        nl.push_back("plane");
        nl.push_back("color");
        //nl.push_back("rotor"); /// Хммммммммммммм раскоментарить и динамический убъется

        if(name == "crow")
        {
            const double scale = /*0.2*/ 0.035;
            pat->setScale(osg::Vec3(scale,scale,scale));
            pat->setAttitude(osg::Quat(osg::inDegrees(90.0),osg::X_AXIS));
        }

        MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,sp::_2,name,sp::_3,sp::_4),/*nullptr*//*[=](osg::Node* model,std::string mat_name){}*/creators::computeAttributes,utils::singleton<mat::reader>::instance().read(mat_file_name));
        pat->accept(mv);
        pat->setName("pat");
        

        FIXME(Свойства моделей на хардкоде);
#if 1
        if(name=="towbar")
        {
           auto towpoint =  findFirstNode(object_file,"tow_point"); 

           pat->setPosition(osg::Vec3(0,zm/2,towpoint->asTransform()?-towpoint->asTransform()->asMatrixTransform()->getMatrix().getTrans().z():0));
           
        }
#endif


		objCache[name] = object = new Object(*pat);

		if(data)
		{
			const xml_model_t::animations_t&  anims = data->anims;
			for(auto it = anims.begin();it!= anims.end();++it)
			{
				const std::string anim_file_name = osgDB::findFileInPath(it->second, fpl.fpl_,osgDB::CASE_INSENSITIVE);
				if(!anim_file_name.empty())
					object->addAnimation(it->first,osgDB::readNodeFile(anim_file_name));
			}
		}

#if 1
        if(fclone )
        {
            object = /*dynamic_cast<osg::PositionAttitudeTransform *>*/(osg::clone(objCache[name].get(), copyop ));
        }
        else
            pat = dynamic_cast<osg::PositionAttitudeTransform *>(objCache[name]->getNode());
#endif


	}

    pat->setNodeMask( PICK_NODE_MASK | REFLECTION_MASK );

	return object;
}

}
