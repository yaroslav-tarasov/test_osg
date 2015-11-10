#pragma once

#include "objects/registrator.h"

namespace aircraft_reg
{

struct settings_t
{
    settings_t()
        : preset_path("presets/ada/default.ada")
    {
    }

    std::string preset_path;

    REFL_INNER(settings_t)
        REFL_ENTRY_RO(preset_path)
    REFL_END()
};

struct view
    : base_view_presentation    
    , info                      
    , obj_data_holder<wrap_settings<settings_t>>
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    view(kernel::object_create_t const& oc, dict_copt dict);    

private:
    bool add_aircraft(aircraft::info_ptr airc_info);

private:
    void on_object_created(object_info_ptr object);
    void on_object_destroying(object_info_ptr object);

    // info
private:


protected:
    typedef size_t  normal_id_t;

    std::unordered_map<normal_id_t, aircraft::info_ptr>  aircrafts_;
};

} // end of aircraft_reg