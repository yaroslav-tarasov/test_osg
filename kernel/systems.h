#pragma once
#include "kernel_fwd.h"
#include "application/document_fwd.h"
#include "av/avVisualFwd.h"

#include "common/dyn_lib.h"
#include "attributes/attr_common.h"

#include "reflection/reflection_ext.h"

#if 0
#include "vcs_settings.h"
#endif


//
namespace net_layer
{
    struct ses_srv;
}

namespace kernel
{

// properties
struct vis_sys_props
{
    struct vis_channel_t
    {
        vis_channel_t()
            : hfov       (60)
            , hdeflection(0)
            , course     (0)
            , local      (false)
            , pixel_scale(1.)

            , cylindric_geom_corr(false)
        {
        }

        double  hfov;
        double  hdeflection;
        double  course;
        bool    local; 
        double  pixel_scale;

        std::string camera_name;

        bool cylindric_geom_corr;
    };

    struct window_t
    {
        window_t()
            : size       (500, 500)
            , fullscreen (false)
        {
        }

        point_2i ltcorner;
        point_2i size    ;
        bool     fullscreen;        
    };

    vis_sys_props()
        : base_point(55.9724, 37.4131, 0) // DEBUG: KTA Sheremetyevo airport 
        //: base_point(43.44444, 39.94694,0)  
    {
    }
    
    geo_point_3     base_point;
    vis_channel_t   channel;
    window_t        window;
};

REFL_STRUCT(kernel::vis_sys_props::vis_channel_t)
    REFL_NUM    (hfov        ,    5 , 160., .05)
    REFL_NUM    (hdeflection , -180., 180., .05)
    REFL_NUM    (course      , -180., 180., .05)
    REFL_NUM    (pixel_scale ,    .5,   5.,  .1)
    REFL_ENTRY  (camera_name)
    REFL_ENTRY  (cylindric_geom_corr)
REFL_END()

REFL_STRUCT(kernel::vis_sys_props::window_t)
    REFL_ENTRY(ltcorner)
    REFL_ENTRY(size) 
    REFL_ENTRY(fullscreen)
REFL_END()

REFL_STRUCT(kernel::vis_sys_props)
    REFL_ENTRY (base_point)
    REFL_ENTRY (channel   )    
    REFL_ENTRY (window    )
REFL_END()

// via nfi

//! интерфейс фабрики подсистем; создание различных подсистем
struct systems_factory
{
    virtual ~systems_factory(){}

    virtual system_ptr       create_model_system    (msg_service& service, std::string const& script)                            = 0;
    virtual system_ptr       create_visual_system   (msg_service& service, av::IVisualPtr vis,    vis_sys_props const&)          = 0;

    virtual system_ptr       create_ctrl_system  (msg_service& service)                                                          = 0;                              
};

typedef boost::shared_ptr<systems_factory> systems_factory_ptr;

// --call via nfi from systems library 
// systems_factory_ptr create_system_factory();

} // kernel
