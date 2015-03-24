#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace vehicle
{
    struct control;

    typedef polymorph_ptr<control>              control_ptr;
}