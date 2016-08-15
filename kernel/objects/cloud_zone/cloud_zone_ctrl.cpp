#include "precompiled_objects.h"
#include "cloud_zone_ctrl.h"

#include "common/cloud_zone.h"

namespace cloud_zone
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}

AUTO_REG_NAME(cloud_zone_ext_ctrl, ctrl::create);

ctrl::ctrl(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, *dict)

    , airports_manager_(find_first_object<airports_manager::info_ptr>(collection_))
{
    // This subscription is to avoid update before all airports are loaded
    conn_holder() << dynamic_cast<system_session*>(oc.sys)->subscribe_session_loaded(boost::bind(&ctrl::on_settings_changed, this));
}

ctrl::~ctrl()
{
}

void ctrl::update( double time )
{
    view::update(time) ;
    // TODO : move on update
}

void ctrl::on_settings_changed()
{

}

} // end of cloud_zone



