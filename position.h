#pragma once

//! позиция в декартовом пространстве
struct decart_position
{
    decart_position() {}

    decart_position(point_3 const& pos, quaternion const& orien)
        : pos(pos)
        , orien(orien)
    {}

    decart_position(point_3 const& pos, point_3 const& dpos, quaternion const& orien, point_3 const& omega)
        : pos(pos)
        , dpos(dpos)
        , orien(orien)
        , omega(omega)
    {}

    point_3    pos;
    point_3    dpos;
    quaternion orien;
    point_3    omega;
};

inline decart_position operator *(transform_4 const& tr, decart_position const& pos)
{
    decart_position res = pos;
    res.pos   = tr * pos.pos;
    res.dpos  = tr * pos.dpos;
    res.orien = quaternion(tr.rotation().cpr()) * pos.orien;
    res.omega = tr * pos.omega; // TODO

    return res;
}

inline decart_position operator *(decart_position const& pos, transform_4 const& tr)
{
    decart_position res = pos;
    res.pos   = pos.pos + pos.orien.rotate_vector(tr.translation());
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
    {}
    geo_position( geo_point_3 const& pos, point_3 const& dpos, quaternion const& orien, point_3 const& omega )
        : pos(pos), dpos(dpos), orien(orien), omega(omega)
    {}

    geo_position( decart_position const& decart, geo_base_3 const& base )
        : pos (base(decart.pos))
        , dpos(decart.dpos)
        , orien(decart.orien)
        , omega(decart.omega)
    {}

    //! некие параметры ориентации в пространстве

    geo_base_3 pos;
    point_3    dpos;
    quaternion orien;
    point_3    omega;
};


