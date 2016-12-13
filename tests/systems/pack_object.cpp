#include "stdafx.h"
#include "kernel/systems_fwd.h"

#include "kernel/systems/systems_base.h"
#include "kernel/object_class.h"

#include "airport/airport_common.h"

#include "objects_factory.h"
#include "common/ext_msgs.h"
#include "factory_systems.h"
#include "objects/common/net_object_factory.h"

using namespace kernel;



void pack_objects(kernel::system* sys, const net_layer::msg::setup_msg& msg, dict_t& dict)
{
    high_res_timer     hr_timer;

    auto fact = dynamic_cast<objects_factory*>(sys);

    kernel::creating_objects_list_t   obj_list;

    obj_list.push_back(std::move(auto_object::pack( fact,"phys_sys")));
    obj_list.push_back(std::move(auto_object::pack( fact,"airports_manager")));
    obj_list.push_back(std::move(auto_object::pack( fact,"ada")));
    obj_list.push_back(std::move(auto_object::pack( fact,"meteo_proxy")));
    obj_list.push_back(std::move(auto_object::pack( fact,"labels_manager")));
    obj_list.push_back(std::move(auto_object::pack( fact,"aircraft_reg")));
    
    {
        airport::settings_t as;
        if(airport::valid_icao(msg.icao_code))
            as.icao_code = msg.icao_code;
        else
            as.icao_code = airport::get_icao_code(msg.icao_code);

        obj_list.push_back(std::move(airport::pack(fact,as)));
    }
    
    force_log fl;       
    LOG_ODS_MSG( "pack_objects(const std::string& airport): airport::settings_t " << hr_timer.set_point() << "\n");
    
    for (auto it = msg.data.begin(); it != msg.data.end(); ++it)
    {
        net_layer::msg::create_msg m;
        network::safe_read_msg(*it,m);
        obj_list.push_back(pack_object(sys, m).add_data(m.ext_id));
    }
    
    sys->pack_exercise(obj_list, dict, true);

}




