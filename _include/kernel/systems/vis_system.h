#pragma once

//#include "victory/vtVictoryFwd.h"
#include "common/event.h"
#include "kernel/systems.h"

namespace kernel
{

struct visual_object
{
    virtual ~visual_object(){}

    virtual osg::ref_ptr<osg::Node> node() const = 0;
    virtual osg::ref_ptr<osg::Node> root() const = 0;

    virtual void set_visible(bool visible) = 0;
    DECLARE_EVENT(object_loaded, (uint32_t) );
};

struct visual_system
{
    virtual ~visual_system(){}


    virtual visual_object_ptr       create_visual_object(std::string const & res, uint32_t seed = 0) = 0;
    virtual visual_object_ptr       create_visual_object( nm::node_control_ptr parent,std::string const & res, uint32_t seed = 0 ) = 0;
    //virtual victory::IVictoryPtr    victory () = 0;
    //virtual victory::IScenePtr      scene   () = 0;
    //virtual victory::IViewportPtr   viewport() = 0;
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
