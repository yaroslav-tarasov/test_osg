#pragma once

#include "objects/registrator.h"
#include "aircraft_physless.h"

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
    , control
    , obj_data_holder<wrap_settings<settings_t>>
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);
    view(kernel::object_create_t const& oc, dict_copt dict);    

private:
    bool add_aircraft(aircraft_physless::info_ptr airc_info);

private:
    void on_object_created(object_info_ptr object);
    void on_object_destroying(object_info_ptr object);

    // control
private:
    virtual void inject_msg(net_layer::test_msg::run const& msg);

    // info
private:

    // base_presentation
protected:
    void pre_update (double time) override;

protected:
    typedef size_t  normal_id_t;
    typedef size_t  extern_id_t;

    std::unordered_map<normal_id_t, aircraft_physless::info_ptr>  aircrafts_;
    std::unordered_map<extern_id_t, normal_id_t>                  e2n_;
    std::deque<net_layer::test_msg::run>                          buffer_;
};

} // end of aircraft_reg