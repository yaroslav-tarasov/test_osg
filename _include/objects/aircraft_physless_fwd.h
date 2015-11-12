#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace aircraft_physless
{
    struct info;
    struct control;
    struct model_control;


    typedef polymorph_ptr<info>                 info_ptr;
    typedef boost::weak_ptr<info>               info_wptr;
    typedef polymorph_ptr<control>              control_ptr;
    typedef polymorph_ptr<model_control>        model_control_ptr;
}