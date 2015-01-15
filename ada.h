#pragma once

#include "bada/bada_enums.h"

namespace ada
{
typedef bada::engine_kind     engine_kind;
typedef bada::turbulence_kind turbulence_kind;


//#pragma pack(push, 1)

struct data_t
{
    std::string base_model;
    std::string manufacturer;
    std::string model_name;

    double S; // wing area
    double span;
    double length;
    unsigned engine;
    unsigned turbulence;

    // drag coefficients
    double cd_0_cruise;
    double cd_2_cruise;
    double cd_0_approach;
    double cd_2_approach;
    double cd_0_landing;
    double cd_0_landing_gear;
    double cd_2_landing;

    // fuel coefficients
    double cf_1;
    double cf_2;
    double cf_3;
    double cf_4;
    double cf_cruise;

    double nominal_ba_to_ld;
    double nominal_ba;
    double max_ba_to_ld;
    double max_ba_hold;
    double max_ba;

    double max_operating_height;
    double max_height;
    double temperature_gradient;
    double mass_gradient;
    double max_mass;
    double min_mass;
    double max_payload_mass;
    double ref_mass;

    double ct_1;
    double ct_2;
    double ct_3;
    double ct_4;
    double ct_5;
    double ct_descent_low;
    double descent_height_level;
    double ct_descent_high;
    double ct_descent_approach;
    double ct_descent_landing;

    std::array<double, 3> V_1;
    std::array<double, 3> V_2;
    std::array<double, 3> M;

    std::array<double, 8> v_d_cl;
    std::array<double, 7> v_d_des;

    double v_stall_to;
    double v_stall_cr;
    double v_stall_ic;
    double v_stall_ap;
    double v_stall_ld;

    double c_v_min;
    double c_v_min_to;
    double h_descent;

    double v_rw_backtrack;
    double v_taxi;
    double v_apron;
    double v_gate;

    double h_max_to;
    double h_max_ic;
    double h_max_ap;
    double h_max_ld;
    double h_final_landing;
    double h_braking;

    double takeoff_length;
    double landing_length;

    double max_mach;
    double max_cas;

    double max_accel_l;
    double max_accel_n;

    double max_rocd;

    // sample for A306
    data_t()
        : h_final_landing(30)
        , h_braking(0.1)
        , max_accel_l(2 * cg::feet2meter())
        , max_accel_n(5 * cg::feet2meter())
        , ref_mass(0)
        , max_rocd(35)
    {
    //@    LogInfo("ada::data_t");
    }
};    

//#pragma pack(pop)

} // ada