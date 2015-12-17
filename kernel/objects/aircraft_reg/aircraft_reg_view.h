#pragma once

#include "objects/registrator.h"

namespace objects_reg
{

struct settings_t
{
    settings_t()
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


protected:
    typedef size_t  object_id_t;

    std::unordered_map<object_id_t, kernel::object_info_ptr>  objects_;

};

} // end of objects_reg