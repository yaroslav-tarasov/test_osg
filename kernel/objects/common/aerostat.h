#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace aerostat
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
    };


    typedef polymorph_ptr  <info>       info_ptr;
    typedef polymorph_ptr  <vis_info>   vis_info_ptr;
    typedef boost::weak_ptr<vis_info>   vis_info_wptr;
    typedef polymorph_ptr  <control>    control_ptr;


}