#pragma once

namespace phys
{
struct compound_sensor_impl  : public compound_sensor_t
{
    compound_sensor_impl(){}
    compound_sensor_impl(btCompoundShape* s, cg::point_3 offset )
        : cs_(s)
        , offset_(offset)
    {}
    cg::point_3 get_offset(){return offset_;};

    btCompoundShape*        cs_;
    cg::point_3             offset_;
};

typedef polymorph_ptr<compound_sensor_impl> compound_sensor_impl_ptr;
}