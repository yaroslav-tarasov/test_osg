#pragma once

//#include "geo_utils.h"

#include "alloc/pool_stl.h"

namespace cg
{
    template<typename T, typename LERP = default_curve_lerp<T>, typename DIST = details::distance2d_t>
    class point_curve_2_ext
        : public curve_t<T, LERP>
    {
    };

} // namespace cg