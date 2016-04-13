#pragma once

#include "camera_common_fwd.h"

namespace camera_object
{
    struct info
    {
        // virtual geo_point_3 const&     pos() const =0; // FIXME and visual_control
        virtual ~info() {}
    };

    typedef polymorph_ptr<info>     info_ptr;
    typedef boost::weak_ptr<info>  info_wptr;

    struct model_ext_control
    {
        virtual ~model_ext_control() {}
        virtual void                 set_desired   (double time,const cg::point_3& pos, const cg::quaternion& q, const double speed)     = 0;
    };


}