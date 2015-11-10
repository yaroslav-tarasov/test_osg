#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_view.h"


namespace aircraft_reg
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(areg_view, view::create);

view::view( kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{
    if (!dict)
    {
        boost::optional<std::string> default_preset_path = oc.hierarchy_class->find_attribute("preset") ;
        if (default_preset_path)
            settings().preset_path = *default_preset_path ;
    }


}

void view::on_object_created(object_info_ptr object)
{
    if (aircraft::info_ptr airc_info = object)
        add_aircraft(airc_info);
}

void view::on_object_destroying(object_info_ptr object)
{
    if (aircrafts_.find(object->object_id()) != aircrafts_.end())
    {
        aircrafts_.erase(object->object_id());
        //nid2id_.erase(narrow(object->object_id()));
    }
}

bool view::add_aircraft(aircraft::info_ptr airc_info)
{
    size_t id = object_info_ptr(airc_info)->object_id();
    aircrafts_[id] = airc_info;
    // nid2id_[narrow(id)] = id;

    return true;
}

} // end of aircraft