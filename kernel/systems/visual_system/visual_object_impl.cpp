#include "stdafx.h"

#include "kernel/systems/vis_system.h"

#include "visual_object_impl.h"
#include "av/avScene/Scene.h"
#include "av/avCore/Utils.h"
#include "animutils.h"
#include "utils/visitors/cache_nodes_visitor.h"

namespace kernel
{
    struct visual_object_impl::private_t
    {
        private_t()
            : scene_( avScene::GetScene() )
            , anim_manager_ (nullptr) 
        {}

        CacheNodesVisitor::CacheMapType                          cache_;
        osg::observer_ptr<avScene::Scene>                        scene_;
        osg::ref_ptr<osg::Node>                                   node_;
        osg::ref_ptr<osg::Node>                                   root_;
        osg::ref_ptr<osgAnimation::BasicAnimationManager> anim_manager_;
    };

    visual_object_impl::visual_object_impl( std::string const & res, uint32_t seed, bool async )
        : loaded_(false)
        , p_  (make_shared<private_t>())
    {
        seed_ =  seed;

		if(!async)
		{
            p_->node_ = p_->scene_->load(res, nullptr, seed, async);
            p_->root_ = findFirstNode(p_->node_,"root",findNodeVisitor::not_exact);
            loaded_ = true;

#if 1
            using namespace avAnimation;

            AnimationManagerFinder finder;
            p_->node_->accept(finder);
            p_->anim_manager_  = finder._am;  

            if(finder._am.valid())
                p_->node_->setUpdateCallback(finder._am.get());
#endif

#if 0
            SetupRigGeometry switcher(true, *node_.get());
#endif
		}
        else
        {
#ifdef ASYNC_OBJECT_LOADING  
            p_->scene_->subscribe_object_loaded(boost::bind(&visual_object_impl::object_loaded,this,_1));
#endif
            p_->node_ = p_->scene_->load(res, nullptr, seed, async);
        }

    }

    visual_object_impl::visual_object_impl(  nm::node_control_ptr parent, std::string const & res, uint32_t seed, bool async )
        : loaded_(false)
        , p_  (make_shared<private_t>())
    {
#ifdef ASYNC_OBJECT_LOADING           
        p_->scene_->subscribe_object_loaded(boost::bind(&visual_object_impl::object_loaded,this,_1));
        seed_ =  seed;
#endif

        auto const& vn = nm::vis_node_control_ptr(parent)->vis_nodes();
        p_->node_ = p_->scene_->load(res,vn.size()>0?vn[0]:nullptr, seed, async);

		if(!async && p_->node_ )
		{

        p_->root_ = findFirstNode(p_->node_,"root",findNodeVisitor::not_exact);

		using namespace avAnimation;

#if 1
		AnimationManagerFinder finder;
		p_->node_->accept(finder);
		p_->anim_manager_  = finder._am;  

		if(finder._am.valid())
			p_->node_->setUpdateCallback(finder._am.get());
#endif

#if 0
		SetupRigGeometry switcher(true, *node_.get());
#endif
		}
    }

    visual_object_impl::~visual_object_impl()
    {
        if(p_->node_.get())
            Utils::RemoveNodeFromAllParents( p_->node_.get() );
        // scene_->get_objects()->remove(node_.get());
    }

#ifdef ASYNC_OBJECT_LOADING    
    void visual_object_impl::object_loaded( uint32_t seed )
    {
         if(seed_==seed)
         {
           p_->root_ = findFirstNode(p_->node_,"root",findNodeVisitor::not_exact); 
           loaded_ = true;

		   using namespace avAnimation;

		   AnimationManagerFinder finder;
		   p_->node_->accept(finder);

		   if(finder._am.valid())
           {
               p_->anim_manager_  = finder._am;  
			   p_->node_->setUpdateCallback(finder._am.get());
           }

           CacheNodesVisitor cnv(p_->cache_);
           node()->accept(cnv);
#if 0
		   SetupRigGeometry switcher(true, *p_->node_.get());
#endif
           
		   object_loaded_signal_(seed);
         }
    }
#endif

    osg::ref_ptr<osg::Node> visual_object_impl::node() const
    {
        return p_->node_;
    }
    
    osg::ref_ptr<osg::Node> visual_object_impl::root() const
    {
        return p_->root_;
    }
    
    osg::Node* visual_object_impl::get_node(const std::string& name) const 
    {
          CacheNodesVisitor::CacheMapType::iterator it;
          if( (it = p_->cache_.find(name)) != p_->cache_.end())
            return it->second.get();
          else
            return nullptr;
    }

    osg::ref_ptr<osgAnimation::BasicAnimationManager> visual_object_impl::animation_manager() const
    {
        return p_->anim_manager_;
    }

    void visual_object_impl::set_visible(bool visible)
    {
        if(loaded_)
            p_->node_->setNodeMask(visible?/*0xffffffff*/0x00010000:0);
    }
}
