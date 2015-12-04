#pragma once

#include "cpp_utils/polymorph_ptr.h"


namespace vehicle
{

    struct model_info;
    struct model_control;


    typedef polymorph_ptr<model_info>       model_info_ptr;
    typedef polymorph_ptr<model_control>    model_control_ptr;
}