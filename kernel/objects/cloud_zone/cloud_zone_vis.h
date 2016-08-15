#pragma once

#include "cloud_zone_view.h"

#include "common/airports_manager.h" 

#include "kernel/kernel_fwd.h"

namespace cloud_zone
{

struct visual
    : kernel::visual_presentation
    , view
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    ~visual() ;

private:
    visual(kernel::object_create_t const& oc, dict_copt dict);

    // base_presentation
private:
    void update( double time );

    // view
private:
    void on_settings_changed() override ;

private:
    airports_manager::info_ptr airports_manager_ ;

private:
    kernel::visual_system* sys_;
};

} // nodes_management
