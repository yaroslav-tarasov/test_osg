#pragma once

//! позиция в декартовом пространстве
struct decart_position
{
    decart_position() {}

    decart_position(point_3 const& pos, quaternion const& orien)
        : pos  (pos)
        , dpos (point_3(0.0,0.0,0.0))
        , ddpos(point_3(0.0,0.0,0.0))
        , orien(orien)
        , omega(point_3(0.0,0.0,0.0))
    {}

    decart_position(point_3 const& pos, point_3 const& dpos, quaternion const& orien, point_3 const& omega)
        : pos(pos)
        , dpos(dpos)
        , ddpos(point_3(0.0,0.0,0.0))
        , orien(orien)
        , omega(omega)
    {}

    inline void freeze()
    {
        dpos  = point_3();
        ddpos = point_3();
        omega = point_3();
    }
    
    inline point_3 dpos_t( double dt )
    {
        return dpos * dt ;
    }

    inline point_3 ddpos_t( double dt )
    {
        return (dpos + ddpos * dt * .5) * dt ;
    }

    point_3    pos;
    point_3    dpos;
    point_3    ddpos;
    quaternion orien;
    point_3    omega;
};

inline decart_position operator *(transform_4 const& tr, decart_position const& pos)
{
    decart_position res = pos;
    res.pos   = tr * pos.pos;
    res.dpos  = tr * pos.dpos;
    res.ddpos = tr * pos.ddpos;
    res.orien = quaternion(tr.rotation().cpr()) * pos.orien;
    res.omega = tr * pos.omega; // TODO

    return res;
}

inline decart_position operator *(decart_position const& pos, transform_4 const& tr)
{
    decart_position res = pos;
    res.pos   = pos.pos + pos.orien.rotate_vector(tr.translation());
    res.ddpos  = pos.ddpos;
    res.dpos   = pos.dpos;
    res.orien = pos.orien * quaternion(tr.rotation().cpr());
    res.omega = pos.omega;

    return res;
}

//! глобальная географическая позиция в пространстве
struct geo_position
{
    geo_position() {}
    geo_position( geo_point_3 const& pos, quaternion const& orien )
        : pos(pos), orien(orien)
        , dpos (point_3(0.0,0.0,0.0))
        , ddpos(point_3(0.0,0.0,0.0))
        , omega(point_3(0.0,0.0,0.0))
    {}
    geo_position( geo_point_3 const& pos, point_3 const& dpos, quaternion const& orien, point_3 const& omega )
        : pos(pos), dpos(dpos), ddpos(point_3(0.0,0.0,0.0)) , orien(orien), omega(omega)
    {}

    geo_position( decart_position const& decart, geo_base_3 const& base )
        : pos (base(decart.pos))
        , dpos(decart.dpos)
        , ddpos(decart.ddpos)
        , orien(decart.orien)
        , omega(decart.omega)
    {}

    inline void freeze()
    {
        dpos  = point_3();
        ddpos = point_3();
        omega = point_3();
    }

    inline point_3 dpos_t( double dt )
    {
        return (dpos + ddpos * dt * .5 ) * dt ;
    }
    
    inline point_3 ddpos_t( double dt )
    {
        return (dpos + ddpos * dt * .5) * dt ;
    }

    geo_base_3 pos;
    point_3    dpos;
    point_3    ddpos;
    quaternion orien;
    point_3    omega;
};

//! рефлексия на эти структуры - возможность записывать и считывать их из каких-то внешних источников

REFL_STRUCT(geo_position)
    REFL_ENTRY(pos )
    REFL_ENTRY(dpos)
    REFL_ENTRY(ddpos)
    REFL_ENTRY(orien)
    REFL_ENTRY(omega)
REFL_END()

REFL_STRUCT(decart_position)
    REFL_ENTRY(pos)
    REFL_ENTRY(dpos)
    REFL_ENTRY(ddpos)
    REFL_ENTRY(orien)
    REFL_ENTRY(omega)
REFL_END()
