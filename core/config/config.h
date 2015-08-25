#pragma once 
#include "reflection/proc/binary.h"

struct cfg_t
{      
    struct path_t
    {
        path_t()
            : data("data") //data("../data")
        {}

        std::string data;
    };

    struct model_params_t
    {   
        model_params_t()
            : bullet_step (0.01)
            , msys_step   (0.1)
            , vsys_step   (0.01)
            , csys_step   (0.03)
            , vehicle_mass(200.0)
            , rod_mass    (100.0)
        {

        }

        double bullet_step;
        double msys_step;
        double vsys_step;
        double csys_step;
        double vehicle_mass;
        double rod_mass;
    };

    struct debug_t
    {
        debug_t()
            : debug_drawer(false) 
        {}

        bool  debug_drawer;
    };

    struct network_t
    {
        network_t()
            : local_address("127.0.0.1:30000") 
        {}

        std::string  local_address;
    };

    path_t               path;
    model_params_t       model_params;
    debug_t              debug;
    network_t            network;
};

REFL_STRUCT(cfg_t::path_t)
    REFL_ENTRY(data)
REFL_END()

REFL_STRUCT(cfg_t::model_params_t)
    REFL_ENTRY(bullet_step)
    REFL_ENTRY(msys_step)
    REFL_ENTRY(vsys_step)
    REFL_ENTRY(csys_step)
    REFL_ENTRY(vehicle_mass)
    REFL_ENTRY(rod_mass)
REFL_END()

REFL_STRUCT(cfg_t::debug_t)
    REFL_ENTRY(debug_drawer)
REFL_END()

REFL_STRUCT(cfg_t::network_t)
    REFL_ENTRY(local_address)
REFL_END()

REFL_STRUCT(cfg_t)
    REFL_ENTRY(path)
    REFL_ENTRY(model_params)
    REFL_ENTRY(debug)
    REFL_ENTRY(network)
REFL_END()

cfg_t const& cfg();