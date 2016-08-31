#include "stdafx.h"
#include "kernel/systems_fwd.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

#include "common/simple_route.h"
#include "objects/flock_manager/flock_manager_common.h"
#include "common/flock_manager.h"
#include "objects/aerostat/aerostat_common.h"
#include "common/aerostat.h"
#include "vehicle/vehicle_common.h"
#include "airport/airport_common.h"
#include "aircraft/aircraft_common.h"
#include "human/human_common.h"
#include "common/human.h"

#include "common/cloud_zone.h"

#include "common/rocket_flare.h"
#include "rocket_flare/rocket_flare_common.h"

#include "arresting_gear/arresting_gear_common.h"
#include "objects_factory.h"

#include "common/ext_msgs.h"
#include "utils/krv_import.h"


namespace {

using namespace kernel;
using namespace net_layer::msg;

inline object_info_ptr create_aircraft(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name; 
    as.custom_label = msg.custom_label;
    return  aircraft::create(dynamic_cast<fake_objects_factory*>(csys),as, gp);
}

inline kernel::obj_create_data pack_aircraft(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name; 
    as.custom_label = msg.custom_label;
    return  aircraft::pack(dynamic_cast<fake_objects_factory*>(csys),as, gp);
}

inline object_info_ptr create_helicopter_phl(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name;  
    as.custom_label = msg.custom_label;

    return  helicopter_physless::create(dynamic_cast<fake_objects_factory*>(csys),as,gp);
}

inline kernel::obj_create_data pack_helicopter_phl(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name;  
    as.custom_label = msg.custom_label;

    return  helicopter_physless::pack(dynamic_cast<fake_objects_factory*>(csys),as,gp);
}

inline object_info_ptr create_aircraft_phl(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name; // "A319"; //
    as.custom_label = msg.custom_label;

    return  aircraft_physless::create(dynamic_cast<fake_objects_factory*>(csys),as,gp);
}

inline kernel::obj_create_data pack_aircraft_phl(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aircraft::settings_t as;
    as.kind = msg.model_name; // "A319"; //
    as.custom_label = msg.custom_label;

    return  aircraft_physless::pack(dynamic_cast<fake_objects_factory*>(csys),as,gp);
}

inline object_info_ptr create_vehicle(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    vehicle::settings_t vs;
    vs.model        = msg.model_name;
    vs.custom_label = msg.custom_label;

    return  vehicle::create(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}

inline kernel::obj_create_data pack_vehicle(kernel::system* csys,create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    vehicle::settings_t vs;
    vs.model        = msg.model_name;
    vs.custom_label = msg.custom_label;

    return  vehicle::pack(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}



inline object_info_ptr create_flock_of_birds(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());
    
    flock::manager::settings_t vs;
    vs.model        = "crow";
    vs._childAmount = msg.num_instances;

    return flock::manager::create(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}

inline kernel::obj_create_data pack_flock_of_birds(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    flock::manager::settings_t vs;
    vs.model        = "crow";
    vs._childAmount = msg.num_instances;

    return flock::manager::pack(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}


inline object_info_ptr create_character(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    human::settings_t vs;
    vs.model = msg.model_name;

    return human::create(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}

inline kernel::obj_create_data pack_character(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    human::settings_t vs;
    vs.model = msg.model_name;

    return human::pack(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}
 
inline object_info_ptr create_arresting_gear(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    arresting_gear::settings_t ms;
    // ms.model = "arresting_gear";

    return arresting_gear::create(dynamic_cast<fake_objects_factory*>(csys),ms,gp);
}

inline kernel::obj_create_data pack_arresting_gear(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    arresting_gear::settings_t ms;
    // ms.model = "arresting_gear";

    return arresting_gear::pack(dynamic_cast<fake_objects_factory*>(csys),ms,gp);
}

inline object_info_ptr create_aerostat(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
	geo_position gp(dpos, get_base());

	aerostat::settings_t ms;
	ms.model = "aerostat";

	return aerostat::create(dynamic_cast<fake_objects_factory*>(csys),ms,gp);
}

inline kernel::obj_create_data pack_aerostat(kernel::system* csys, create_msg const& msg)
{
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    aerostat::settings_t ms;
    ms.model = "aerostat";

    return aerostat::pack(dynamic_cast<fake_objects_factory*>(csys),ms,gp);
}

inline object_info_ptr create_camera(kernel::system* csys, create_msg const& msg)
{       
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

	return camera::create(dynamic_cast<fake_objects_factory*>(csys), gp ,  msg.model_name);
}

inline kernel::obj_create_data pack_camera(kernel::system* csys, create_msg const& msg)
{       
    decart_position dpos(msg.pos,msg.orien);
    geo_position gp(dpos, get_base());

    return camera::pack(dynamic_cast<fake_objects_factory*>(csys), gp ,  msg.model_name);
}

object_info_ptr create_cloud_zone(kernel::system* csys, update_cloud_zone_msg const& msg)
{
    return  cloud_zone::create(dynamic_cast<fake_objects_factory*>(csys), msg.settings);
}


inline object_info_ptr create_rocket_flare(kernel::system* csys, create_msg const& msg)
{
	decart_position dpos(msg.pos,msg.orien);
	geo_position gp(dpos, get_base());

	rocket_flare::settings_t vs;

	return rocket_flare::create(dynamic_cast<fake_objects_factory*>(csys),vs,gp);
}

object_info_ptr create_object( kernel::system* csys, create_msg const& msg)
{
    if(msg.object_kind & ok_vehicle)
        return create_vehicle(csys, msg);
    else if ( msg.object_kind == ok_flock_of_birds)
        return create_flock_of_birds(csys, msg);
    else if ( msg.object_kind ==ok_human)
        return create_character(csys, msg);
    else if ( msg.object_kind == ok_helicopter)
        return create_helicopter_phl(csys, msg); 
	else if ( msg.object_kind == ok_rocket_flare)
		return create_rocket_flare(csys, msg); 
    else if( msg.object_kind == ok_camera)
        return create_camera(csys, msg);
    else
        return create_aircraft(csys, msg); // create_aircraft_phl(csys, msg);  // FIXME вместо чекера можно создать какой-нибудь более дурной объект

}

kernel::obj_create_data pack_object( kernel::system* csys, create_msg const& msg)
{
    if(msg.object_kind & ok_vehicle)
        return pack_vehicle(csys, msg);
    else if ( msg.object_kind == ok_flock_of_birds)
        return pack_flock_of_birds(csys, msg);
    else if ( msg.object_kind ==ok_human)
        return pack_character(csys, msg);
    else if ( msg.object_kind == ok_helicopter)
        return pack_helicopter_phl(csys, msg); 
    else if( msg.object_kind == ok_camera)
        return pack_camera(csys, msg);
    else
        return pack_aircraft(csys, msg); // pack_aircraft_phl(csys, msg);  // FIXME вместо чекера можно создать какой-нибудь более дурной объект

}

}



AUTO_REG_NAME(create_cloud_zone, create_cloud_zone)

AUTO_REG_NAME(create_object, create_object)
AUTO_REG_NAME(pack_object, pack_object)

