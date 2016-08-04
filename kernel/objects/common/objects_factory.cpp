#include "stdafx.h"
#include "precompiled_objects.h"

#include "kernel/systems/systems_base.h"

#include "concept_object/concept_object.h"

#include "aircraft/aircraft_view.h"
#include "aircraft_fms/aircraft_fms_view.h"
#include "aircraft_physless/aircraft_physless_view.h"
#include "helicopter_physless/helicopter_physless_view.h"
#include "vehicle/vehicle_view.h"
#include "simple_route/simple_route_view.h"
#include "airport/airport_view.h"
#include "environment/environment_view.h"
#include "flock_manager/flock_manager_view.h"
#include "arresting_gear/arresting_gear_view.h"
#include "aerostat/aerostat_view.h"
#include "human/human_view.h"
#include "camera/camera_view.h"
#include "rocket_flare/rocket_flare_view.h"

#include "nodes_manager/nodes_manager_view.h"
#include "kernel/systems/fake_system.h"

#include "objects/objects_factory.h"

namespace 
{
    using namespace kernel;

    std::list<obj_create_data> creating_objects_list_t;

    inline object_info_ptr sys_object_create(fake_objects_factory* sys, obj_create_data const& descr)
    {
         return sys->create_object(descr);
    }
}


namespace aircraft
{

using namespace kernel;

    obj_create_data pack(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        const std::string class_name = "aircraft";
        const std::string unique_name = sys->generate_unique_name(class_name);
        obj_create_data ocd(class_name, unique_name, dict::wrap(aircraft::craft_data(sett,state_t(init_pos.pos,init_pos.orien)/*, fpl_id*/)));

        ocd
            .add_child(obj_create_data("fms"          , "fms"          , dict::wrap(aircraft::aircraft_fms::craft_fms_data())))
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())))
            //.add_child(obj_create_data("gui"          , "gui"       , dict::wrap(aircraft::aircraft_gui::gui_data      ())));
            ;

        return ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        return sys_object_create(sys, pack( sys, sett, init_pos));	
    }

}

namespace flock
{
    namespace manager
    {
        obj_create_data pack(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
        {
            const std::string class_name = "flock_manager";
            const std::string unique_name = sys->generate_unique_name(class_name);

            return obj_create_data(class_name, unique_name, dict::wrap(manager_data(sett, state_t(init_pos.pos, init_pos.orien))));	
        }

        object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
        {
            return sys_object_create(sys, pack( sys, sett, init_pos));	
        }
    }
}

namespace human
{

    obj_create_data pack(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        const std::string class_name = "human";
        const std::string unique_name = sys->generate_unique_name(class_name);
        
        FIXME(Скорость присутствует)
        
        obj_create_data ocd(class_name, unique_name, dict::wrap(human_data(sett, state_t(init_pos.pos, init_pos.orien,cg::norm(init_pos.dpos)))));
        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data  ())));

        return ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        return sys_object_create(sys, pack( sys, sett, init_pos));	
    }
}

namespace aerostat
{

	obj_create_data pack(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
	{
		const std::string class_name = "aerostat";
		const std::string unique_name = sys->generate_unique_name(class_name);

		obj_create_data ocd(class_name, unique_name, dict::wrap(aerostat_data(sett, state_t(init_pos.pos, init_pos.orien))));
		ocd
			.add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data  ())));

		return ocd;	
	}

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
         return sys_object_create(sys, pack( sys, sett, init_pos));	
    }

}

namespace vehicle
{
    using namespace kernel;

    obj_create_data pack(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        const std::string class_name = "vehicle";
        const std::string unique_name = sys->generate_unique_name(class_name);

        obj_create_data ocd(class_name, unique_name, dict::wrap(vehicle_data(sett, state_t(init_pos.pos, init_pos.orien.get_course(), 10))));
        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data    ())));

        return ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        return sys_object_create(sys, pack( sys, sett, init_pos));	
    }
}


namespace simple_route
{
    using namespace kernel;

    obj_create_data pack(fake_objects_factory* sys,const settings_t& sett,const cg::geo_point_3& init_pos)
    {
        const std::string class_name = "simple_route";
        const std::string unique_name = sys->generate_unique_name(class_name);
        anchor_points_t  aps;
        aps.push_back(anchor_point_t(ani::point_pos(0,init_pos)));
        obj_create_data ocd(class_name, unique_name, dict::wrap(route_data(sett,aps)));

        return  ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const cg::geo_point_3& init_pos)
    {
        return sys_object_create(sys, pack( sys, sett, init_pos));	
    }
}


namespace aircraft_physless
{

    using namespace kernel;
    
    obj_create_data pack(fake_objects_factory* sys,const aircraft::settings_t& sett,const geo_position& init_pos)
    {
        const std::string class_name = "aircraft_physless";
        const std::string unique_name = sys->generate_unique_name(class_name);
        aircraft::state_t s(init_pos.pos,init_pos.orien);
        obj_create_data ocd(class_name, unique_name, dict::wrap(aircraft_physless::craft_data(sett,s)));

        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data  ())))
            ;

        return ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const aircraft::settings_t& sett,const geo_position& init_pos)
    {
        return sys_object_create(sys, aircraft_physless::pack( sys, sett, init_pos));	
    }

}

namespace helicopter_physless
{

    using namespace kernel;

    obj_create_data pack(fake_objects_factory* sys,const aircraft::settings_t& sett,const geo_position& init_pos)
    {
        const std::string class_name = "helicopter_physless";
        const std::string unique_name = sys->generate_unique_name(class_name);
        aircraft::state_t s(init_pos.pos,init_pos.orien);
        obj_create_data ocd(class_name, unique_name, dict::wrap(helicopter_physless::craft_data(sett,s)));

        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data ())))
            ;

        return  ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const aircraft::settings_t& sett,const geo_position& init_pos)
    {
        return sys_object_create(sys, helicopter_physless::pack( sys, sett, init_pos));
    }
}


namespace airport
{

    using namespace kernel;

    obj_create_data pack(fake_objects_factory* sys,const settings_t& sett)
    {
        const std::string class_name = "airport";
        const std::string unique_name = sys->generate_unique_name(class_name);
        atc::airport::data_t  data;
        obj_create_data ocd(class_name, unique_name, dict::wrap(airport::port_data(sett,data)));

        ocd
            .add_child(obj_create_data("nodes_manager" , "nodes_manager" , dict::wrap(nodes_management::nodes_data ())))
			.add_child(obj_create_data("environment"   , "environment"   , dict::wrap(environment::settings_t      ())))
            .add_child(obj_create_data("arresting_gear", "arresting_gear", dict::wrap(arresting_gear::settings_t   ())))
            ;

        return  ocd;	
    }

    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett)
    {
        return sys_object_create(sys, pack( sys, sett));
    }
}

namespace camera
{
	using namespace kernel;

	obj_create_data pack(fake_objects_factory* sys,const geo_position& init_pos, boost::optional<std::string> name)
	{
		const std::string class_name = "camera";
		const std::string unique_name = name?*name:sys->generate_unique_name(class_name);
		// camera_object::binoculars_t  data;
        camera_object::camera_data  data;
        data.cam_orien = init_pos.orien;
        data.cam_pos   = init_pos.pos;
		obj_create_data ocd(class_name, unique_name, dict::wrap(data));

		ocd
			.add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data ())))
			;

		return  ocd;	
	}

    object_info_ptr create(fake_objects_factory* sys,const geo_position& init_pos, boost::optional<std::string> name)
    {
        return sys_object_create(sys, pack( sys,  init_pos, name));
    }
}

namespace rocket_flare
{
	using namespace kernel;

	obj_create_data pack(fake_objects_factory* sys,const settings_t& sett, const geo_position& init_pos)
	{
		const std::string class_name = "rocket_flare";
		const std::string unique_name = sys->generate_unique_name(class_name);

		rocket_flare::state_t s(init_pos.pos,init_pos.orien);
		return obj_create_data(class_name, unique_name, dict::wrap(rocket_flare_data(sett,s)));	
	}

	object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
	{
		return sys_object_create(sys, pack( sys, sett, init_pos));	
	}

}


namespace auto_object
{

    obj_create_data pack(fake_objects_factory* sys, const std::string& class_name)
    {
        const std::string unique_name = sys->generate_unique_name(class_name);

        return obj_create_data(class_name, /*unique_name*/class_name);	
    }

    object_info_ptr create(fake_objects_factory* sys, const std::string& class_name)
    {
        return sys_object_create(sys, pack( sys, class_name));	
    }

}