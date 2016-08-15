#pragma once

#include "cpp_utils/polymorph_ptr.h"
#include "meteo/meteo.h"

namespace cloud_zone
{

enum cloud_kind
{
    PK_NONE,

    PK_FOG,
    PK_RAIN,
    PK_SNOW,
    PK_HAIL,
    PK_THUNDER,

    PK_NO_PRECIPITATION,
};

struct settings_t
{
    typedef std::vector<cg::geo_point_2>  points_t;

    settings_t(points_t const& points)
        : heights(200., 400.)
        , kind   (PK_NONE)
        , points (points)
        , inten  (0)
    {}

    settings_t(points_t const&& points)
        : heights(200., 400.)
        , kind   (PK_NONE)
        , points (std::move(points))
        , inten  (0)
    {}

    settings_t()
        : heights(200., 400.)
        , kind   (PK_NONE)
        , inten  (0)
    {}

    cg::range_2    heights;
    cg::range_2ui  time;
    unsigned       kind;
    points_t       points;
    double         inten;
};


struct info
    : meteo::cloud_contour_t
{
    virtual ~info()
    { }

    virtual settings_t const&  settings    () const = 0;

    virtual bool  is_inside(geo_point_3 const& pos) const = 0;
};

struct chart_control
{
    virtual ~chart_control() { }

    virtual void in_conflict(bool in_cnfl) = 0;
};

typedef polymorph_ptr<info>           info_ptr;
typedef polymorph_ptr<chart_control>  chart_control_ptr;

REFL_STRUCT(settings_t)
    REFL_ENTRY(heights)
    REFL_ENTRY(time)
    REFL_ENUM (kind, ("None", "Fog", "Rain", "Snow", "Hail", "Thunder", "No precipitation", nullptr))
    REFL_SER  (points)
    REFL_NUM  (inten, 0.0, 1.0, 0.01)
REFL_END()

} // namespace cloud_zone



