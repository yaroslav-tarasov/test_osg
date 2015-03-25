#include "stdafx.h"
#include "precompiled_objects.h"

#include "kernel/systems/systems_base.h"

#include "concept_object/concept_object.h"

#include "aircraft/aircraft_view.h"
#include "aircraft_fms/aircraft_fms_view.h"

#include "vehicle/vehicle_view.h"

#include "simple_route/simple_route_view.h"

#include "nodes_manager/nodes_manager_view.h"
#include "fake_system.h"

#include "objects/object_creators.h"

namespace aircraft
{

using namespace kernel;

object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const cg::geo_point_3& init_pos)
{
    const std::string class_name = "aircraft";
    FIXME("”никальное им€")
    const std::string unique_name = "aircraft_0";
    obj_create_data ocd(class_name, unique_name, dict::wrap(aircraft::craft_data(sett/*, fpl_id*/)));

    ocd
        .add_child(obj_create_data("fms"          , "fms"          , dict::wrap(aircraft::aircraft_fms::craft_fms_data())))
        .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())))
        ;//.add_child(obj_create_data("gui"          , "gui"          , dict::wrap(aircraft::aircraft_gui::gui_data      ())));

    return sys->create_object(ocd);	
}

}


namespace vehicle
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const cg::geo_point_3& init_pos)
    {
        const std::string class_name = "vehicle";
        FIXME("”никальное им€")
        const std::string unique_name = "vehicle_0";
        
        const double course = 30;

        obj_create_data ocd(class_name, unique_name, dict::wrap(vehicle_data(sett, state_t(init_pos, course, 10))));
        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())));

        return sys->create_object(ocd);	
    }
}

namespace simple_route
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const cg::geo_point_3& init_pos)
    {
        const std::string class_name = "simple_route";
        FIXME("”никальное им€")
        const std::string unique_name = "simple_route_0";
        anchor_points_t  aps;
        aps.push_back(anchor_point_t(ani::point_pos(0,init_pos)));
        obj_create_data ocd(class_name, unique_name, dict::wrap(route_data(sett,aps)));

        return sys->create_object(ocd);	
    }
}