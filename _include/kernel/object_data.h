#pragma once
#include "kernel/kernel_fwd.h"
#include "kernel/object_data_fwd.h"

namespace kernel
{

    typedef uint32_t object_data_t;

    struct object_data
    {
        virtual const object_data_t&          get_data      () const = 0;
        virtual ~object_data(){}
    };


} // kernel
