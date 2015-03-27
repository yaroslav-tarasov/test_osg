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
            : bullet_step(0.01)
            , msys_step  (0.1)
            , vsys_step  (0.01)
            , csys_step  (0.03)
        {

        }

        double bullet_step;
        double msys_step;
        double vsys_step;
        double csys_step;
    };

    path_t               path;
    model_params_t       model_params;
};

REFL_STRUCT(cfg_t::path_t)
    REFL_ENTRY(data)
REFL_END()

REFL_STRUCT(cfg_t::model_params_t)
    REFL_ENTRY(bullet_step)
    REFL_ENTRY(msys_step)
    REFL_ENTRY(vsys_step)
    REFL_ENTRY(csys_step)
REFL_END()

REFL_STRUCT(cfg_t)
    REFL_ENTRY(path)
    REFL_ENTRY(model_params)
REFL_END()

cfg_t const& cfg();