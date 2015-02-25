#pragma once
#include "cpp_utils/inplace_format.h"

namespace fpl
{

inline settings_base_t::settings_base_t()
    : flight_number(333)
    , company_code ("AFL")
    , reg_number   ("SAM")
    , aircraft_type("B737")
    , flight_rules (1)
    , turbulence   (bada::medium)
    , depart_time  (0)
    , dest_time    (depart_time + 3 * 3600)
    , rfl          (9000.)
    , cruise_speed (600.)
    , equipment    (atc::OE_W)
    , ssr_code     (atc::RC_UNDEFINED)
{
}

inline settings_base_t::settings_base_t(concept_object::afpl const& co_afpl)
    // co_afpl.name is saved into view
    : flight_number (co_afpl.flight_number)
    , company_code  (co_afpl.company_code)
    , reg_number    (co_afpl.reg_number)

    , aircraft_type (co_afpl.aircraft_type)
    , flight_rules  (co_afpl.flight_rules)
    , turbulence    (co_afpl.turbulence)

    //TODO : deduce correct time for all imported plans
    //one should put date in ex settings
    , depart_time   (co_afpl.depart_time)
    , dest_time     (co_afpl.dest_time)

    , depart_airport(co_afpl.depart_airport)
    , dest_airport  (co_afpl.dest_airport)

    , dest_airport_1(co_afpl.dest_airport_1)
    , dest_airport_2(co_afpl.dest_airport_2)

    , rfl           (co_afpl.cruise_level)
    , cruise_speed  (co_afpl.cruise_speed)

    , equipment     (atc::OE_W)
    , ssr_code      (atc::RC_UNDEFINED)
{
}

inline std::string settings_base_t::acid() const
{
    return str(cpp_utils::inplace_format("%s%d") % company_code % flight_number);
}

inline settings_t::settings_t()
    : payload(1)
    , fuelload(1)
{
}

inline settings_t::settings_t(concept_object::afpl const& co_afpl)
    : settings_base_t(co_afpl)
    , payload(1)
    , fuelload(1)
{
}

inline settings_t::settings_t(settings_base_t const& base)
    : settings_base_t(base)
    , payload(1)
    , fuelload(1)
{
}

REFL_STRUCT(settings_base_t)
    REFL_NUM(flight_number, 0, 10000, 1)

    REFL_ENTRY(company_code)
    REFL_ENTRY(reg_number)
    REFL_ENTRY(aircraft_type)

    REFL_ENUM(flight_rules, ("I (Instrumental)", "V (Visual)", "Y (Start instrumental)", "Z (Start visual)", NULL))
    REFL_ENUM(turbulence, ("L (Light)", "M (Medium)", "H (Heavy)", "J (Jumbo)", NULL))

    REFL_ENTRY(depart_airport)
    REFL_ENTRY(depart_parking)
    REFL_TIME(depart_time)

    REFL_ENTRY(dest_airport)
    REFL_ENTRY(dest_parking)
    REFL_TIME_RO(dest_time)

    REFL_ENTRY(dest_airport_1)
    REFL_ENTRY(dest_airport_2)

    REFL_NUM(rfl, 0, 25000, 100)
    REFL_NUM(cruise_speed, 10, 1000, 10)

    REFL_FLAG_ENUM(equipment, ("W (RVSM)", NULL))
    REFL_NUM      (ssr_code, 1000, 99999, 1)
REFL_END()

REFL_STRUCT(settings_t)
    REFL_CHAIN(settings_base_t)

    REFL_ENTRY(depart_runway)
    REFL_ENTRY(dest_runway)

    REFL_ENTRY(sid_name)
    REFL_ENTRY(star_name)

    REFL_ENTRY(payload)
    REFL_ENTRY(fuelload)
REFL_END()

} // namespace fpl


