
#pragma once 

namespace vehicle
{
namespace msg 
{
//! идентификаторы подвижных объектов
enum id 
{
    vm_state,
    vm_phys_pos,
    vm_settings,

    vm_go_to_pos,
    vm_follow_route,
    vm_attach_tow,
    vm_detach_tow,

    vm_debug_controls,
    vm_disable_debug_controls,

    vm_tow,

    vm_brake
};

typedef gen_msg<vm_settings, settings_t>    settings_msg_t;
typedef gen_msg<vm_attach_tow, uint32_t>    attach_tow_msg_t;
typedef gen_msg<vm_detach_tow, void>        detach_tow_msg_t;
typedef gen_msg<vm_follow_route, uint32_t>  follow_route_msg_t;
typedef gen_msg<vm_disable_debug_controls>  disable_debug_ctrl_msg_t;
typedef gen_msg<vm_tow, boost::optional<uint32_t>> tow_msg_t;
typedef gen_msg<vm_brake, double> brake_msg_t;

struct state_msg_t
    : network::msg_id<vm_state>
{               
    state_msg_t()
    {
    }

    state_msg_t(state_t const& state)
        : pos(state.pos)
        , course((float)state.course)
        , speed((float)state.speed)
    {
    }

    operator state_t() const
    {
        return state_t(pos, course, speed);
    }

    cg::geo_point_2 pos;
    double course;
    double speed;
};

REFL_STRUCT(state_msg_t)
    REFL_ENTRY(pos)
    REFL_ENTRY(course)
    REFL_ENTRY(speed)
REFL_END   ()

//! сообщение - что-то отладочное (?)
struct debug_controls_data
    : network::msg_id<vm_debug_controls>
{
    debug_controls_data(double thrust = 0, double steer = 0, double brake = 0)
        : thrust(thrust)
        , steer (steer )
        , brake (brake )
    {}

    double thrust;
    double steer;
    double brake;
};           

REFL_STRUCT(debug_controls_data)
    REFL_ENTRY(thrust)
    REFL_ENTRY(steer)
    REFL_ENTRY(brake)
REFL_END()

//! сообщение - позиция подвижного объекта
struct phys_pos_msg
    : network::msg_id<vm_phys_pos>
{
    phys_pos_msg() {}


    phys_pos_msg( cg::geo_point_3 pos, double course)
        : pos(pos), course(course)
    {}

     cg::geo_point_3 pos;
    double course;
};

REFL_STRUCT(phys_pos_msg)
    REFL_ENTRY(pos)
    REFL_ENTRY(course)
REFL_END()

//! сообщение - перемещение подвижного объекта к позиции
struct go_to_pos_data
    : network::msg_id<vm_go_to_pos>
{
    go_to_pos_data() {}

    go_to_pos_data( cg::geo_point_2 pos, double course)
        : pos(pos), course(course)
    {}

     cg::geo_point_2 pos;
    double course;
};

REFL_STRUCT(go_to_pos_data)
    REFL_ENTRY(pos)
    REFL_ENTRY(course)
REFL_END()

} // msg
} // namespace vehicle

