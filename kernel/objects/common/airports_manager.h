#pragma once

#include "common/airport.h"

namespace airports_manager
{
    struct info
    {
        virtual ~info() {}
        
        virtual airport::info_ptr find_closest_airport(geo_point_2 const& pos) const = 0;
    };

    typedef polymorph_ptr<info> info_ptr;
}