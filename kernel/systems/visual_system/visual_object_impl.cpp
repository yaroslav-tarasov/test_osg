#include "stdafx.h"

#include "kernel/systems/vis_system.h"

#include "visual_object_impl.h"
#include "av/avScene/Scene.h"
#include "av/avCore/Utils.h"
#include "animutils.h"

namespace kernel
{
    visual_object_impl::visual_object_impl( std::string const & res, uint32_t seed, bool async )
        : scene_( avScene::GetScene() )
        , loaded_(false)
        , anim_manager_ (nullptr)
    {
#ifdef ASYNC_OBJECT_LOADING
        scene_->subscribe_object_loaded(boost::bind(&visual_object_impl::object_loaded,this,_1));
        node_ = scene_->load(res, nullptr, seed, async);
        seed_ =  seed;
#endif
		if(!async)
		{
        node_ = scene_->load(res, nullptr, seed, async);
        root_ = findFirstNode(node_,"root",findNodeVisitor::not_exact);
        loaded_ = true;

#if 1
        using namespace avAnimation;

        AnimationManagerFinder finder;
        node_->accept(finder);
        anim_manager_  = finder._am;  

        if(finder._am.valid())
            node_->setUpdateCallback(finder._am.get());
#endif

#if 0
        SetupRigGeometry switcher(true, *node_.get());
#endif
		}     

    }

    visual_object_impl::visual_object_impl(  nm::node_control_ptr parent, std::string const & res, uint32_t seed, bool async )
                        : scene_ ( avScene::GetScene() )
                        , parent_(parent)
                        , loaded_(false)
    {
#ifdef ASYNC_OBJECT_LOADING           
        scene_->subscribe_object_loaded(boost::bind(&visual_object_impl::object_loaded,this,_1));
        seed_ =  seed;
#endif

        auto const& vn = nm::vis_node_control_ptr(parent)->vis_nodes();
        node_ = scene_->load(res,vn.size()>0?vn[0]:nullptr, seed, async);

		if(!async)
		{

        root_ = findFirstNode(node_,"root",findNodeVisitor::not_exact);

		using namespace avAnimation;

#if 1
		AnimationManagerFinder finder;
		node_->accept(finder);
		anim_manager_  = finder._am;  

		if(finder._am.valid())
			node_->setUpdateCallback(finder._am.get());
#endif

#if 0
		SetupRigGeometry switcher(true, *node_.get());
#endif
		}
    }

    visual_object_impl::~visual_object_impl()
    {
        // scene_->removeChild(node_.get());
        utils::RemoveNodeFromAllParents( node_.get() );
        // scene_->get_objects()->remove(node_.get());
    }

#ifdef ASYNC_OBJECT_LOADING    
    void visual_object_impl::object_loaded( uint32_t seed )
    {
         if(seed_==seed)
         {
           root_ = findFirstNode(node_,"root",findNodeVisitor::not_exact); 
           loaded_ = true;

		   using namespace avAnimation;

		   AnimationManagerFinder finder;
		   node_->accept(finder);

		   if(finder._am.valid())
           {
               anim_manager_  = finder._am;  
			   node_->setUpdateCallback(finder._am.get());
           }

           object_loaded_signal_(seed);
         }
    }
#endif

    osg::ref_ptr<osg::Node> visual_object_impl::node() const
    {
        return node_;
    }
    
    osg::ref_ptr<osg::Node> visual_object_impl::root() const
    {
        return root_;
    }
    
    osg::ref_ptr<osgAnimation::BasicAnimationManager> visual_object_impl::animation_manager() const
    {
        return anim_manager_;
    }

    void visual_object_impl::set_visible(bool visible)
    {
        if(loaded_)
            node_->setNodeMask(visible?/*0xffffffff*/0x00010000:0);
    }
}
