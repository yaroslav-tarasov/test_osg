#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace environment
{
    struct info;
    struct control;

    typedef polymorph_ptr<info>                 info_ptr;
    typedef boost::weak_ptr<info>               info_wptr;
    typedef polymorph_ptr<control>              control_ptr;

}