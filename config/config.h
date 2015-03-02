#pragma once 
#include "reflection/proc/binary.h"

struct cfg_t
{      
    struct path_t
    {
        path_t()
            : data("../data")
        {}

        std::string data;
    };

    path_t               path;
};

REFL_STRUCT(cfg_t::path_t)
    REFL_ENTRY(data)
REFL_END()

REFL_STRUCT(cfg_t)
    REFL_ENTRY(path)
REFL_END()

cfg_t const& cfg();