#include "stdafx.h"
#include "precompiled_objects.h"

#include "aircraft_reg_view.h"


namespace aircraft_reg
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(aircraft_reg_view, view::create);

view::view( kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{
}

void view::on_object_created(object_info_ptr object)
{
    if (aircraft_physless::info_ptr airc_info = object)
        if(airc_info)
            add_aircraft(airc_info);
}

void view::on_object_destroying(object_info_ptr object)
{
    auto a = aircrafts_.find(object->object_id());
    if ( a != aircrafts_.end())
    {
        e2n_.erase(aircraft_physless::info_ptr(a->second)->extern_id());
        aircrafts_.erase(object->object_id());
        //nid2id_.erase(narrow(object->object_id()));
    }
}

bool view::add_aircraft(aircraft_physless::info_ptr airc_info)
{
    size_t id = object_info_ptr(airc_info)->object_id();
    size_t eid = airc_info->extern_id();

    aircrafts_[id] = airc_info;
    e2n_[eid]      = id;

    // nid2id_[narrow(id)] = id;

    return true;
}

void view::inject_msg(net_layer::test_msg::run const& msg)
{
    // set(msg);
}


} // end of aircraft_reg