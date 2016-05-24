#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace arresting_gear
{

    struct info
    {
        virtual ~info() {}

        virtual geo_point_3 pos() const = 0;
        virtual std::string const& name() const = 0;
    };

    struct vis_info
    {
        virtual ~vis_info() {}

        virtual bool is_visible() const = 0 ;
    };
    
    struct control
    {
        virtual ~control(){};
        virtual void set_target(const boost::optional<uint32_t>&   ext_id)=0;
    };


    typedef polymorph_ptr  <info>       info_ptr;
    typedef polymorph_ptr  <vis_info>   vis_info_ptr;
    typedef boost::weak_ptr<vis_info>   vis_info_wptr;
    typedef polymorph_ptr  <control>    control_ptr;


}