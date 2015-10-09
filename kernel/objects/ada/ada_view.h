#pragma once

//#include "ada_transliterator.h"
#include "objects/ada.h"

//! представление характеристик ВС

namespace ada
{

//! "настройки" для параметров ВС - по сути путь к параметрам ВС по умолчанию
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

//! вид; по сути - непосредственно представление характеристик всех ВС
struct view
    : base_view_presentation    // базовый класс
    , info                      // интерфейс получения данных по имени типа ВС
    , obj_data_holder<wrap_settings<settings_t>>
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    view(kernel::object_create_t const& oc, dict_copt dict);    

    // info
private:
    optional<data_t const&> get_data(string const& aircraft_kind) const override;

protected:
    void save_preset(string const& path);
    void load_preset(string const& path);

protected:
    // transliterator_t transliterator_;
    mutable std::map<string, data_t> aircrafts_;
};

} // end of ada