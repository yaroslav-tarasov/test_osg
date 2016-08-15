#include "stdafx.h"
#include "av/precompiled.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "Object.h"
#include "av/avCore/LOD.h"
#include "av/avLights/Lights.h"
#include "av/avLights/LightManager.h"
#include "av/avCore/Utils.h"
#include "av/avCore/XmlModel.h"
#include "av/avCore/InstancesManager.h"

#include "materials.h"

#include "animutils.h"


namespace avCore
{
    ObjectManager& ObjectManager::get()
    {
        static ObjectManager  m;
        return m;
    }

    void ObjectManager::RegisterPrototype(const std::string& name, Object* obj )
    {
        objCache_[name] = obj;
    }

    void ObjectManager::RegisterClone( Object* obj )
    {
        objClones_.push_back(obj);
    }

    boost::optional<ObjectMap::value_type> ObjectManager::Find(const std::string& name)
    {
        auto it = objCache_.find(name);
        if (objCache_.end()!=it)
            return *it;

        return boost::none;
    }
	
	void ObjectManager::releaseObject(uint32_t seed)
	{
		auto it = std::find_if(objClones_.begin(),objClones_.end(),[=](const osg::ref_ptr<Object> obj)->bool {return obj->getSeed()==seed; });
		if (it!=objClones_.end())
		{
			(*it).release();
			objClones_.erase(it);
		}

	}

    void ObjectManager::releaseAll()
    {
        objCache_.clear();
        objClones_.clear();
    }
    
    bool ObjectManager::PreUpdate()
    {
        for ( ObjectMap::iterator cIt = objCache_.begin(); cIt != objCache_.end(); ++cIt )
        {
            if ( cIt->second->PreUpdate() == false )
                return false;
        } 
         return true;
    }

    void  releaseObjectCache()
    {
        ObjectManager::get().releaseAll();
    }

// constructor and destructor
Object::Object()
{
}

Object::Object(osg::Node& node , const std::string  & name, bool fhw_inst)
    : _node (&node)
    , _name(name) 
{
	if(!fhw_inst)
    {
        using namespace avAnimation;
        AnimationManagerFinder finder;
        _node->accept(finder);
        _manager  = finder.getBM();
    }

    ObjectManager::get().RegisterPrototype(name,this);
}

Object::Object(const Object& object,const osg::CopyOp& copyop)
	: osg::Object(object,copyop)
	, _node            (copyop.getCopyFlags() == osg::CopyOp::SHALLOW_COPY?nullptr:copyop(object._node.get()))
	, _anim_containers (copyop.getCopyFlags() == osg::CopyOp::SHALLOW_COPY?AnimationContainersType():object._anim_containers)
	, _manager         (copyop.getCopyFlags() == osg::CopyOp::SHALLOW_COPY?nullptr:object._manager)
    , _inst_manager    (object._inst_manager)
    , _name            (object._name)
{
     ObjectManager::get().RegisterClone(this);
}


void  Object::addAnimation(const std::string& name, osg::Node* anim_container)
{
    using namespace avAnimation;
    AnimationManagerFinder finder;
    anim_container->accept(finder);

    osgAnimation::BasicAnimationManager* model = finder.getBM();
	
	_anim_containers.insert(make_pair(name,anim_container));
	
	for (osgAnimation::AnimationList::const_iterator it = model->getAnimationList().begin(); it != model->getAnimationList().end(); it++)
	{
        avAnimation::deleteUseless(*(*it).get());
	    
        (*it)->setName(name);
		_animations[ name ] = *it;
		_manager->registerAnimation(*it);

	}
}

void  Object::addAnimation(const std::string& hw_anim_file)
{
     setupInstancedHWAnimated(hw_anim_file);
}

void  Object::setupInstancedHWAnimated(const std::string& hw_anim_file)
{
    if(!(_inst_manager.valid()))
    {
        ::Database::fpl_wrap fpl(_name);
        
        std::string anim_file_name =  osgDB::findFileInPath(hw_anim_file, fpl.get_file_list(),osgDB::CASE_INSENSITIVE);

        _inst_manager = CreateInstancesManager(/*HW_DEFAULT*/HW_ANIMATED,_node.get(), anim_file_name); 
        
        // Двойная регистрация кэш и клоны, для спец узла (последствия?)
        ObjectManager::get().RegisterClone(this);
    }
}

static OpenThreads::Mutex& getInstanceMutex()
{
    static OpenThreads::Mutex   _mutex;
    return _mutex;
}

bool   Object::hwInstanced() const 
{
    return _inst_manager.valid();
}

// FIXME брррррррр
bool   Object::parentMainInstancedNode(osg::Group* parent) 
{
    if(_inst_manager.valid())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getInstanceMutex());
        if(_inst_manager->getMainNode()->getNumParents()==0)
            parent->addChild(_inst_manager->getMainNode());

        return true;
    }

    return false;
}

osg::Node*   Object::getOrCreateNode() 
{
    if(_inst_manager.valid())
    {
       return _inst_manager->getObjectInstance();
    }
    else
        return _node.get(); 
}

bool Object::PreUpdate()
{
	if(_inst_manager.valid() && _node.valid())
	{
		_inst_manager->commitInstancesPositions();
	}
    return true;
}


static OpenThreads::Mutex& getMutex()
{
	static OpenThreads::Mutex   _mutex;
	return _mutex;
}

static OpenThreads::Mutex& getReadFileMutex()
{
	static OpenThreads::Mutex   _mutex;
	return _mutex;
}

struct helper_t
{
    helper_t(const std::string& name)
        : name (name)
    {}
    
    ~helper_t()
    {
		force_log fl;

		LOG_ODS_MSG( "createObject : "  << name << "  time: "<< hr_timer.set_point() << "\n");
    }
    
    high_res_timer hr_timer;
    std::string    name;
};

void processMorphAnimation( boost::optional<xml_model_t> data, osg::Node* object_file )
{
    if(data->morphs.size()>0)
    {
        FindNodeVisitor::nodeNamesList list_name;
        const morph_params& mp = data->morphs["rotor_morph"];
#if 0
        list_name.push_back(mp.parent);
        list_name.push_back(mp.source);
        list_name.push_back(mp.target);
#endif
        FindNodeVisitor findParents(mp.parent,FindNodeVisitor::not_exact); 
        object_file->accept(findParents);

        auto const& nl =  findParents.getNodeList();
        int count_lod0 =  std::count_if(nl.begin(),nl.end(),
            [&](const osg::Node* nl)->bool {
                std::string ls = boost::to_lower_copy(nl->getName());
                return ls.find("lod0") !=std::string::npos;
        });

        for(int i=0;i<count_lod0;i++)
        {
            auto source = findFirstNode(findParents.getNodeList()[i],mp.source,FindNodeVisitor::not_exact);
            auto target = findFirstNode(findParents.getNodeList()[i],mp.target,FindNodeVisitor::not_exact);

            source->setNodeMask(0);
            target->setNodeMask(0);

            FindNodeByType< osg::Geode> geode_source_finder;  
            geode_source_finder.apply(*source);
            osg::Geode*    geode_source = dynamic_cast<osg::Geode*>(geode_source_finder.getLast()); 

            FindNodeByType< osg::Geode> geode_target_finder;  
            geode_target_finder.apply(*target);
            osg::Geode*    geode_target = dynamic_cast<osg::Geode*>(geode_target_finder.getLast()); 

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            for(int gc =0 ; gc<  geode_source->getNumDrawables(); ++gc)
            {
                osg::Geometry* geo_mesh_source = geode_source->getDrawable(gc)->asGeometry();
                osg::Geometry* geo_mesh_target = geode_target->getDrawable(gc)->asGeometry();

                osg::ref_ptr<osgAnimation::MorphGeometry> morph =
                    new osgAnimation::MorphGeometry( *geo_mesh_source );

                morph->addMorphTarget( geo_mesh_target, 0.0);    
                morph->setName("rotor_morph");
                geode->addDrawable( morph.get() );
            }


            geode->setUpdateCallback( new osgAnimation::UpdateMorph("MorphCallback") );
            geode->setName("rotor_morph");

            findParents.getNodeList()[i]->asGroup()->addChild(geode);
        }

    }
}



Object* CreatePrototype( std::string name, bool fclone, osg::CopyOp copyop ) 
{
    boost::optional<xml_model_t> data;
    osg::Node* object_file = nullptr;
    Object*  object = nullptr;

    ::Database::fpl_wrap fpl(name);
    std::string object_file_name =  osgDB::findFileInPath(name + ".xml", fpl.get_file_list(),osgDB::CASE_INSENSITIVE);
    std::string mat_file_name = osgDB::findFileInPath(name+".mat.xml", fpl.get_file_list(),osgDB::CASE_INSENSITIVE);

    if(object_file_name.empty())
    {
        object_file_name = osgDB::findFileInPath(name+".osgb", fpl.get_file_list(),osgDB::CASE_INSENSITIVE);

        if(object_file_name.empty())
            object_file_name = osgDB::findFileInPath(name+".fbx", fpl.get_file_list(),osgDB::CASE_INSENSITIVE);

        if(object_file_name.empty())
            object_file_name = osgDB::findFileInPath(name+".dae", fpl.get_file_list(),osgDB::CASE_INSENSITIVE);

        if(object_file_name.empty())
            return nullptr;

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getReadFileMutex());
        object_file = osgDB::readNodeFile(object_file_name);

    }
    else
    {
        ModelReader mr;
        data =  xml_model_t();
        mr.Load(object_file_name, *data);
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getReadFileMutex());
        object_file = osgDB::readNodeFile(osgDB::findFileInPath((*data).main_model, fpl.get_file_list(),osgDB::CASE_INSENSITIVE));
#if 1
        processMorphAnimation(data, object_file);
#endif
    }

    bool airplane = findFirstNode(object_file ,"shassi_"  ,FindNodeVisitor::not_exact)!=nullptr;
    bool vehicle  = findFirstNode(object_file ,"wheel"    ,FindNodeVisitor::not_exact)!=nullptr;
    FIXME(Палец пол и потолок при определении модели)
        bool heli     = findFirstNode(object_file ,"tailrotor",FindNodeVisitor::not_exact)!=nullptr;


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

    //
    //  Здесь были огни
    //

    //
    //  А здесь будет чертов некошерный лод
    //
    osg::Node* lod_ =  findFirstNode(object_file,"lod_",FindNodeVisitor::not_exact);

    if(lod_ && lod0 && lod3 && (data?data->lod3:true))  // Далеко не все модели имеют заданную структуру ха
    {
        avLod::LOD* lod = new avLod::LOD;

        lod_->asGroup()->addChild(lod);
        lod->addChild(lod0,0,1200);
        lod->addChild(lod3,1200,50000);
    }
    else
    {
        if(lod3)
            lod3->setNodeMask(0);

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

    float xm = abs(bb.xMax()) + abs(bb.xMin());
    float ym = abs(bb.yMax()) + abs(bb.yMin());
    float zm = abs(bb.zMax()) + abs(bb.zMin());

    //float dx = abs(bb.xMax()) - xm / 2.f;
    //float dy = abs(bb.yMax()) - ym / 2.f;
    //float dz = abs(bb.zMax()) - zm / 2.f;

    float dx = -xm / 4.f; 
    float dy = -ym / 4.f; 
    float dz = -zm / 4.f; 

    osg::PositionAttitudeTransform* pat;

    if (object_file->asTransform() && object_file->asTransform()->asPositionAttitudeTransform())
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

    if(data)
    {
        pat->setScale(osg::Vec3(data->scale,data->scale,data->scale));
    }

    if(data)
        if(data->axis_up == xml_model_t::Y_UP)
        {
            pat->setAttitude(osg::Quat(osg::inDegrees(90.0),osg::X_AXIS));
        }
        else if(data->axis_up == xml_model_t::NEG_Y_UP)
        {
            pat->setAttitude(osg::Quat(osg::inDegrees(-90.0),osg::X_AXIS));
        }
        else if(data->axis_up == xml_model_t::NEG_Z_UP)
        {	
            pat->setPivotPoint(data->pivot_point);
            pat->setAttitude(osg::Quat(osg::inDegrees(180.0),osg::X_AXIS));
        }

        MaterialVisitor::namesList nl;
        nl.push_back("building");
        nl.push_back("default");
        nl.push_back("plane");
        nl.push_back("color");
        nl.push_back("tree");
        nl.push_back("rotor"); 

        MaterialVisitor mv ( nl, std::bind(&creators::createMaterial,sp::_1,sp::_2,name,sp::_3,(data && data->hw_instanced && data->hw_data.size()==0)?"inst":"",sp::_4),/*nullptr*//*[=](osg::Node* model,std::string mat_name){}*/creators::computeAttributes,utils::singleton<mat::reader>::instance().read(mat_file_name));
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

        object = new Object(*pat, name, (data && data->hw_instanced) );

        if(data)
        {			
            
            if(!data->hw_instanced)
            {
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getReadFileMutex());
                    const xml_model_t::animations_t&  anims = data->anims;
                    for(auto it = anims.begin();it!= anims.end();++it)
                    {
                        const std::string anim_file_name = osgDB::findFileInPath(it->second, fpl.get_file_list(),osgDB::CASE_INSENSITIVE);
                        if(!anim_file_name.empty())
                            object->addAnimation(it->first,osgDB::readNodeFile(anim_file_name));
                    }
                }

                if(fclone)
                {
                    object = osg::clone(object, copyop );
                }
            }
            else
            {
                  object->addAnimation(data->hw_data);
            }

            
        }

        return  object;
}

ObjectControl* createObject(std::string name, uint32_t seed, bool fclone)
{
	Object*    object      = nullptr;

    //helper_t h(name);

    osg::CopyOp copyop;

    if (name=="crow" || name=="human" )
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
			//& ~osg::CopyOp::DEEP_COPY_CALLBACKS  // wo anim
            ;
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getMutex());

	auto it = ObjectManager::get().Find(name);

	if( it )
	{
        if(it->second.get()->hwInstanced())
        {
            object = osg::clone(it->second.get());
        }
        else
        {
            if(fclone)
            {
                object = osg::clone(it->second.get(), copyop);
            }
            else
                object = it->second.get();
        }
	}
	else
	{
        object = CreatePrototype(name, fclone, copyop);
	}
	
    if(!object->hwInstanced())
    {
        FIXME("human тоже хочет hardware, надо помочь")
        if ( name =="human" )
            avAnimation::SetupRigGeometry switcher(false, *object->getNode(),
            [=]()->osgAnimation::RigTransformHardware* { return new osgAnimation::RigTransformHardware;});
        else
            avAnimation::SetupRigGeometry switcher(true, *object->getNode());
    }


    if(object->getNode())
        object->getNode()->setNodeMask( PICK_NODE_MASK | REFLECTION_MASK );

	object->setSeed(seed);

	return object;
}

void     releaseObject( osg::Node* node )
{
	size_t object_id = 0;
	node->getUserValue("id",object_id);	 
	Utils::RemoveNodeFromAllParents( node );	

	ObjectManager::get().releaseObject( object_id );
}


}
