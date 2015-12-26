#include "stdafx.h"
#include "precompiled_objects.h"

#include "kernel/systems/systems_base.h"

#include "concept_object/concept_object.h"

#include "aircraft/aircraft_view.h"
#include "aircraft_fms/aircraft_fms_view.h"
#include "aircraft_physless/aircraft_physless_view.h"
#include "vehicle/vehicle_view.h"
#include "simple_route/simple_route_view.h"
#include "airport/airport_view.h"
#include "flock_manager/flock_manager_view.h"

#include "nodes_manager/nodes_manager_view.h"
#include "kernel/systems/fake_system.h"

#include "objects/object_creators.h"

namespace aircraft
{

using namespace kernel;

object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
{
    const std::string class_name = "aircraft";
    const std::string unique_name = sys->generate_unique_name(class_name);
    obj_create_data ocd(class_name, unique_name, dict::wrap(aircraft::craft_data(sett,state_t(init_pos.pos,init_pos.orien)/*, fpl_id*/)));

    ocd
        .add_child(obj_create_data("fms"          , "fms"          , dict::wrap(aircraft::aircraft_fms::craft_fms_data())))
        .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())))
        ;//.add_child(obj_create_data("gui"          , "gui"          , dict::wrap(aircraft::aircraft_gui::gui_data      ())));

    return sys->create_object(ocd);	
}

}

namespace flock
{
    namespace manager 
    {
        object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
        {
            const std::string class_name = "flock_manager";
            const std::string unique_name = sys->generate_unique_name(class_name);

            obj_create_data ocd(class_name, unique_name, dict::wrap(manager_data(sett, state_t(init_pos.pos, init_pos.orien))));
            return sys->create_object(ocd);	
        }
    }
}


namespace vehicle
{
    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const /*cg::geo_point_3*/geo_position& init_pos)
    {
        const std::string class_name = "vehicle";
        const std::string unique_name = sys->generate_unique_name(class_name);

        obj_create_data ocd(class_name, unique_name, dict::wrap(vehicle_data(sett, state_t(init_pos.pos, init_pos.orien.get_course(), 10))));
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
        const std::string unique_name = sys->generate_unique_name(class_name);
        anchor_points_t  aps;
        aps.push_back(anchor_point_t(ani::point_pos(0,init_pos)));
        obj_create_data ocd(class_name, unique_name, dict::wrap(route_data(sett,aps)));

        return sys->create_object(ocd);	
    }
}


namespace aircraft_physless
{

    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const aircraft::settings_t& sett,const /*cg::geo_point_3*/geo_position& init_pos)
    {
        const std::string class_name = "aircraft_physless";
        const std::string unique_name = sys->generate_unique_name(class_name);
        aircraft::state_t s(init_pos.pos,init_pos.orien);
        obj_create_data ocd(class_name, unique_name, dict::wrap(aircraft_physless::craft_data(sett,s)));

        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())))
            ;

        return sys->create_object(ocd);	
    }

}


namespace airport
{

    using namespace kernel;

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett)
    {
        const std::string class_name = "airport";
        const std::string unique_name = sys->generate_unique_name(class_name);
        atc::airport::data_t  data;
        obj_create_data ocd(class_name, unique_name, dict::wrap(airport::port_data(sett,data)));

        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())))
            ;

        return sys->create_object(ocd);	
    }

}