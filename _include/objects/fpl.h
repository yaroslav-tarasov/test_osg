#pragma once

#include "fpl_fwd.h"

#include "aircraft_fwd.h"
#include "aircraft_atc.h"

#include "ani/ani_traj_navigation.h"
#include "bada/bada_enums.h"
#include "fms/fms.h"
#include "common/event.h"
#include "atc/atc.h"

namespace fpl
{

//! некая точка (привязки?)
struct anchor_point_t
{
    anchor_point_t() 
    {
    };

    explicit anchor_point_t(ani::point_pos const& pos)
        : pos(pos)
    {
    }

    ani::point_pos pos;
    fms::constraint_t constraint;
    
    REFL_INNER(anchor_point_t)
        REFL_ENTRY(pos)
        REFL_ENTRY(constraint)
    REFL_END()
};

//! базовые настройки самолета из плана
struct settings_base_t
{    
    settings_base_t();
    explicit settings_base_t(concept_object::afpl const& co_afpl);

    unsigned int    flight_number;
    std::string     company_code;   // aircraft company code
    std::string     reg_number;     // on board number

    std::string     aircraft_type;  // ICAO aircraft type
    unsigned        flight_rules;
    unsigned        turbulence;

    std::string     depart_airport;  // ICAO airport code
    std::string     depart_parking;
    unsigned int    depart_time;     // from exercise start

    std::string     dest_airport;    // ICAO airport code
    std::string     dest_parking;
    unsigned int    dest_time;       // from exercise start

    std::string     dest_airport_1;  // alternate airport ICAO airport code
    std::string     dest_airport_2;  // alternate airport ICAO airport code

    double          rfl;             // requested flight level (RFL) m
    double          cruise_speed;    // requested cruise speed m/s

    unsigned        equipment;       // declared equipment
    unsigned        ssr_code;        // declared responder code

    // TODO: 
    //" M/V/N Признак статуса Military (M), или признак литерности рейса (А, В, V), 
    // или количество ВС в группе при групповом полете (N)" - из описания расширенного формуляра в РЭ владивосток

    std::string acid() const ;
};

//! настройки самолета по плану (параметры из плана, аэропорты, полосы взлета и посадки, сид и стар, загрузка и масса топлива)
struct settings_t 
    : settings_base_t
{    
    settings_t();

    explicit settings_t(concept_object::afpl const& co_afpl);
    explicit settings_t(settings_base_t const& base);

    std::string     depart_runway;
    std::string     dest_runway;
    std::string     sid_name;
    std::string     star_name;

    double payload;
    double fuelload;
};

struct info
{
    virtual ~info(){}

    virtual range_2                           flight_period() const = 0;
    virtual bool                              active     ()   const = 0;
    virtual boost::optional<uint32_t> const & assigned   ()   const = 0;
    virtual aircraft::info_ptr                get_assigned_aircraft () const = 0;
    virtual settings_t const &                settings   ()   const = 0;
    virtual fms::traj_collection_ptr          traj       ()   const = 0;
    virtual ani::traj_navigation_ptr          traj_navi()     const = 0;
    virtual vector<ani::point_pos>            anchor_points() const = 0;
    virtual fms::pilot_state_t                get_initial_state() const = 0;
    virtual bool                              contains_point(ani::point_pos const& pos) = 0;
    virtual optional<ani::point_pos>          get_point_by_name(string const& name) = 0;

    DECLARE_EVENT(changed, ());
    DECLARE_EVENT(procedure_changed, ());
};

} // end of fpl

#include "fpl.hpp"