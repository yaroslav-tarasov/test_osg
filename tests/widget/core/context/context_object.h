#pragma once

#include "../gl/helpers.h"

namespace core
{

struct context_object
{
    context_object()
        : context_(core::create_context())
    {
    }

protected:
    context_ptr context_;
};

}
