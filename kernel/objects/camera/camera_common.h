#pragma once 
#include "common/points.h"
#include "geometry/camera.h"
#include "network/msg_base.h"

#include "objects/nodes_management.h"

using namespace kernel;

namespace camera_object
{

struct binoculars_t 
{
    bool   active;
    cpr    orien;
    string target;
    double zoom ;

    binoculars_t()
        : active(false)
        , zoom  (1.)
    {
    }

    bool operator==(binoculars_t const& rhs) const
    {
        return active == rhs.active     && 
               cg::eq(orien, rhs.orien) && 
               target == rhs.target     && 
               cg::eq(zoom , rhs.zoom);
    }

    bool operator!=(binoculars_t const& rhs) const 
    {
        return !(*this == rhs);
    }
};

REFL_STRUCT(binoculars_t)
    REFL_ENTRY(active)
    REFL_ENTRY(orien )
    REFL_ENTRY(target)
    REFL_NUM  (zoom, .1, 40., .1)
REFL_END()

struct camera_data
{
    binoculars_t      bins;
    quaternion   cam_orien;
    geo_point_3    cam_pos;
};

REFL_STRUCT(camera_data)
    REFL_ENTRY(bins)
    REFL_ENTRY(cam_orien )
    REFL_ENTRY(cam_pos)
REFL_END()


namespace msg 
{
//! сообщение бинокля
enum num_id
{
    ni_binocular,
	ni_traj_assign
};

//! тело сообщения бинокля
struct binoculars_msg
    : network::msg_id<ni_binocular>
{
    binoculars_msg()
    {
    }

    binoculars_msg(binoculars_t const& binoculars)
        : binoculars(binoculars)
    {
    }

    binoculars_t binoculars;
};

REFL_STRUCT(binoculars_msg)
    REFL_ENTRY(binoculars)
REFL_END()

struct traj_assign_msg
	: network::msg_id<ni_traj_assign>
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

} // namespace msg

namespace utils
{
//! поиск среди нод нужной ноды    
inline nodes_management::manager_ptr obj_manager(kernel::object_info_ptr obj)
{
    if (obj)
    {
        object_info_vector const& children = object_info_ptr(obj)->objects();
        for (auto it = children.begin(); it!= children.end(); ++it)
            if (nodes_management::manager_ptr mng = *it)
                return mng;
    }

    return nullptr;
}

//! корень
inline nodes_management::node_info_ptr obj_root(object_info_ptr obj)
{
    if (nodes_management::manager_ptr mng = obj_manager(obj))
        return mng->get_node(0);

    return nullptr;
}

} // utils
} // namespace camera_object


