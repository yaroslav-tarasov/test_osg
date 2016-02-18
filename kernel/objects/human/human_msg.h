
#pragma once 

namespace human
{
namespace msg 
{
//! идентификаторы подвижных объектов
enum id 
{
    hm_state,
    hm_phys_pos,
    hm_settings,

    hm_go_to_pos,
    hm_follow_route,

    hm_traj_assign,
    hm_follow_trajectory
};

typedef gen_msg<hm_settings, settings_t>    settings_msg_t;
typedef gen_msg<hm_follow_route, uint32_t>  follow_route_msg_t;
typedef gen_msg<hm_follow_trajectory, uint32_t>  follow_trajectory_msg_t;

REFL_STRUCT(state_t)
	REFL_ENTRY(pos)
	REFL_ENTRY(orien)
REFL_END()

struct state_msg_t
    : network::msg_id<hm_state>
{               
    state_msg_t()
    {
    }

    state_msg_t(state_t const& state)
        : pos  (state.pos)
        , orien(state.orien)
		, speed(state.speed)
    {
    }

    operator state_t() const
    {
        return state_t(pos, orien, speed);
    }

    cg::geo_point_3 pos;
    cg::quaternion  orien;
	double          speed;
};

REFL_STRUCT(state_msg_t)
    REFL_ENTRY(pos)
    REFL_ENTRY(orien)
	REFL_ENTRY(speed)
REFL_END   ()

       

//! сообщение - позиция подвижного объекта
struct phys_pos_msg
    : network::msg_id<hm_phys_pos>
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
    : network::msg_id<hm_go_to_pos>
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

//! сообщение 
struct traj_assign_msg
    : network::msg_id<hm_traj_assign>
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
} // namespace human

