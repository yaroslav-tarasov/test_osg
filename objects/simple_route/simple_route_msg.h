
#pragma once 

namespace simple_route
{
namespace msg 
{

enum id 
{
    vm_add_point,
    vm_settings
};


typedef gen_msg<vm_settings, settings_t>    settings_msg_t;


struct add_point_msg
    : network::msg_id<vm_add_point>
{
    add_point_msg() {}


    add_point_msg( size_t          idx,ani::point_pos pos)
        : idx(idx), pos(pos)
    {}

    size_t          idx;
    ani::point_pos pos;

};

REFL_STRUCT(add_point_msg)
    REFL_ENTRY(idx)
    REFL_ENTRY(pos)
REFL_END()

} // msg
} // namespace simple_route

