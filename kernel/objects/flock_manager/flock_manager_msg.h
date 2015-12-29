
#pragma once 

namespace flock
{

namespace manager
{

namespace msg 
{

enum id 
{
    vm_state,
    vm_phys_pos,
    vm_settings,

    vm_go_to_pos,
    vm_follow_route,

    vm_traj_assign,
    vm_follow_trajectory
};

typedef gen_msg<vm_settings         , settings_t>  settings_msg_t;
typedef gen_msg<vm_follow_route     , uint32_t  >  follow_route_msg_t;
typedef gen_msg<vm_follow_trajectory, uint32_t  >  follow_trajectory_msg_t;


struct state_msg_t
    : network::msg_id<vm_state>
{               
    state_msg_t()
    {
    }

    state_msg_t(state_t const& state)
        : pos(state.pos)
        , orien(state.orien)
    {
    }

    operator state_t() const
    {
        return state_t(pos, orien);
    }

    cg::geo_point_3 pos;
    cg::quaternion  orien;
};

REFL_STRUCT(state_msg_t)
    REFL_ENTRY(pos)
    REFL_ENTRY(orien)
REFL_END   ()

//! сообщение - перемещение подвижного объекта к позиции
struct go_to_pos_data
    : network::msg_id<vm_go_to_pos>
{
    go_to_pos_data() {}

    go_to_pos_data( cg::geo_point_3 pos, double orien)
        : pos(pos), orien(orien)
    {}

    cg::geo_point_3 pos;
    cg::quaternion  orien;
};

REFL_STRUCT(go_to_pos_data)
    REFL_ENTRY(pos)
    REFL_ENTRY(orien)
REFL_END()

//! сообщение 
struct traj_assign_msg
    : network::msg_id<vm_traj_assign>
{
    traj_assign_msg() {}

    traj_assign_msg(const fms::traj_data& traj)
        : traj(traj)
    {}

    traj_assign_msg(const fms::traj_data&& traj)
        : traj(move(traj))
    {}

    fms::traj_data traj;
};

REFL_STRUCT(traj_assign_msg)
    REFL_ENTRY(traj)
REFL_END()

} // msg
} // namespace  manager
} // flock


