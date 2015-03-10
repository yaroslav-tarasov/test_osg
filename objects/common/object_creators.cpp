#include "stdafx.h"
#include "precompiled_objects.h"

#include "kernel/systems/systems_base.h"

#include "concept_object/concept_object.h"

#include "aircraft/aircraft_view.h"
#include "aircraft_fms/aircraft_fms_view.h"
#include "nodes_manager/nodes_manager_view.h"
#include "fake_system.h"

#include "objects/object_creators.h"

namespace aircraft
{

using namespace kernel;

object_info_ptr create(fake_objects_factory* sys,const aircraft::settings_t& sett)
{
    const std::string class_name = "aircraft";
    const std::string unique_name = "aircraft_0";
    obj_create_data ocd("aircraft", /*name*/unique_name, dict::wrap(aircraft::craft_data(sett/*, fpl_id*/)));

    ocd
        .add_child(obj_create_data("fms"          , "fms"          , dict::wrap(aircraft::aircraft_fms::craft_fms_data())))
        .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())))
        ;//.add_child(obj_create_data("gui"          , "gui"          , dict::wrap(aircraft::aircraft_gui::gui_data      ())));

    return sys->create_object(ocd);	
}

}