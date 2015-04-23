#pragma once

#include "ani/ani.h"

namespace dubin_route
{

struct anchor_point_t
{
    anchor_point_t(ani::point_pos const &pos, double speed)
        : pos(pos)
        , speed(speed)
    {}

    anchor_point_t(ani::point_pos const &pos)
        : pos(pos)
        , speed(0)
    {}

    anchor_point_t()
        : speed(0)
    {}


    ani::point_pos pos;
    double speed;
};

typedef 
    std::vector<anchor_point_t>   anchor_points_t;

struct settings_t
{
    settings_t(double speed=0)
        : speed(speed)
    {

    }
    
    double speed;
};

struct info
{
    virtual ~info(){}

    virtual double          length           () const = 0;
    virtual cg::geo_point_2 interpolate      ( double t ) const = 0;  // natural parametrization
    virtual double          interpolate_speed( double t ) const = 0;
    virtual double          closest          ( cg::geo_point_3 const& p ) const = 0;
};

typedef polymorph_ptr<info> info_ptr;

struct control
{
    virtual ~control(){}

    virtual void      add_point( cg::geo_point_3 const& p )  = 0;
    virtual void      set_speed( double  speed )  = 0;
};

typedef polymorph_ptr<control> control_ptr;

REFL_STRUCT(anchor_point_t)
    REFL_ENTRY(pos)
    REFL_NUM(speed, 0., 100., 0.1)
REFL_END()

REFL_STRUCT(settings_t)
    REFL_NUM(speed, 0., 100., 0.1)
REFL_END()

} // namespace dubin_route


