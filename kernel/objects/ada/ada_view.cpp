#include "stdafx.h"
#include "precompiled_objects.h"

#include "ada_view.h"

// #include "config/config.h"
#include "bada/bada_import.h"

namespace ada
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(ada_view, view::create);

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

    load_preset(settings_.preset_path);
}

optional<data_t const&> view::get_data(std::string const& aircraft_kind) const
{
    auto it = aircrafts_.find(/*transliterator_*/(aircraft_kind));
    if (it != aircrafts_.end())
         return it->second;
    //else
    //{
    //    aircrafts_[aircraft_kind] = ada::fill_data("BADA",aircraft_kind);
    //    return aircrafts_[aircraft_kind];
    //}


    return aircrafts_.find("B737")->second;
}

void view::save_preset(string const& path)
{
    if (path.empty())
        return ;

    try
    {
        dict::save_binary(boost::filesystem::path(cfg().path.data) / path.c_str(), aircrafts_) ;
    }
    catch(verify_error const&)
    {
        LogError("Incorrect path to save: " << path) ;
        return ;
    }

    settings_.preset_path = path;
}

void view::load_preset(string const& path)
{
    if (path.empty())
        return ;

    dict::load_binary(boost::filesystem::path(cfg().path.data) / settings_.preset_path, aircrafts_) ;
    settings_.preset_path = path;
}

} // end of ada