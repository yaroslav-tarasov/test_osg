#include "stdafx.h"
#include "kernel/systems_fwd.h"

#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"

FIXME(Это что за нафиг нужно  для object_creators )
#include "common/simple_route.h"
#include "objects/flock_manager/flock_manager_common.h"
#include "common/flock_manager.h"
#include "objects/aerostat/aerostat_common.h"
#include "common/aerostat.h"
#include "vehicle/vehicle_common.h"
#include "airport/airport_common.h"
#include "aircraft/aircraft_common.h"

#include "objects/human/human_common.h"
#include "common/human.h"

#include "arresting_gear/arresting_gear_common.h"

#include "objects_factory.h"

#include "common/ext_msgs.h"

#include "factory_systems.h"

#include "utils/krv_import.h"


using namespace kernel;

namespace 
{
    inline std::string data_file(std::string icao_code)
    {
        if (icao_code == "UUEE")
            return "log_sochi_4.txt";
        else if (icao_code == "URSS")
            return "log_sochi_4.txt";
        else if (icao_code == "UMMS" || icao_code == "UMMS")
            return "log_minsk.txt";

        return "";
    }

void pack_objects(const std::string & airport)
{
    const std::string icao_code = airport; 

    krv::data_getter              _krv_data_getter(data_file(icao_code));
    
    high_res_timer hr_timer;

    // Только получение без контроля  
    kernel::system_ptr _csys = get_systems()->get_control_sys();
    
    kernel::creating_objects_list_t   obj_list;

    {
        airport::settings_t as;
        as.icao_code = icao_code;
        obj_list.push_back(std::move(airport::pack(dynamic_cast<fake_objects_factory*>(kernel::fake_objects_factory_ptr(_csys).get()),as)));
    }
    
    force_log fl;       
    LOG_ODS_MSG( "pack_objects(const std::string& airport): airport::settings_t " << hr_timer.set_point() << "\n");


}

}

AUTO_REG(pack_objects)

