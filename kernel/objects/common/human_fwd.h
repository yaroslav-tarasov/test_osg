#pragma once

#include "cpp_utils/polymorph_ptr.h"


namespace human
{

    struct model_info;
    struct model_control;


    typedef polymorph_ptr<model_info>       model_info_ptr;
    typedef polymorph_ptr<model_control>    model_control_ptr;
}