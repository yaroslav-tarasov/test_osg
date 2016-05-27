#pragma once

#include "av/avVisualFwd.h"
#include "common/event.h"
#include "kernel/systems.h"

namespace kernel
{

struct visual_object
{
    typedef boost::function<void(uint32_t seed)>                   on_object_loaded_f;

    virtual ~visual_object(){}

    virtual osg::ref_ptr<osg::Node> node() const = 0;
    virtual osg::ref_ptr<osg::Node> root() const = 0;
    virtual osg::ref_ptr<osgAnimation::BasicAnimationManager> animation_manager() const = 0;
    virtual osg::Node* get_node(const std::string& name) const = 0;


    virtual void set_visible(bool visible) = 0;
    DECLARE_EVENT(object_loaded, (uint32_t) );
};

struct visual_system
{
    typedef boost::function<void(uint32_t seed)>                   on_object_loaded_f;
    
    virtual ~visual_system(){}

#ifdef ASYNC_OBJECT_LOADING
    virtual visual_object_ptr       create_visual_object(std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0, bool async=true) = 0;
    virtual visual_object_ptr       create_visual_object( nm::node_control_ptr parent,std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0, bool async=true ) = 0;
#else
	virtual visual_object_ptr       create_visual_object(std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0, bool async=false) = 0;
	virtual visual_object_ptr       create_visual_object( nm::node_control_ptr parent,std::string const & res, on_object_loaded_f f = 0, uint32_t seed = 0, bool async=false ) = 0;
#endif

	virtual av::IVisualPtr          visual  () = 0;
    virtual av::IScenePtr           scene   () = 0;
    //virtual av::IViewportPtr      viewport() = 0;
};

struct visual_system_props
{
    virtual ~visual_system_props(){}

    virtual vis_sys_props const&    vis_props   () const               = 0;
    virtual void                    update_props(vis_sys_props const&) = 0;
};

struct visual_presentation
{
    virtual ~visual_presentation(){}
};

struct visual_control
{
    virtual ~visual_control(){}

    virtual geo_point_3 pos  () const = 0;
    virtual cpr         orien() const = 0;
};

} // kernel
