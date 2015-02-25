#pragma once

#include "fms/fms.h"
#include "fms/fms_holding.h"
#include "fms/fms_instruments.h"

#include "ada/ada.h"
#include "common/event.h"

//! структуры и интерфейсы, касающиеся проводки ВС

namespace aircraft
{
namespace aircraft_fms
{

//! состояние ВС, наследуемся от состояния "автопилота" и добавляем крен и тангаж, и какую-то версию(?)
struct state_t : fms::pilot_state_t
{
    state_t();
    state_t(fms::pilot_state_t const& ps, double pitch, double roll, uint32_t version);
    cpr orien() const;

    double pitch;
    double roll;
    uint32_t version;
};

REFL_STRUCT(state_t)
    REFL_CHAIN(fms::pilot_state_t)
    REFL_ENTRY(pitch)
    REFL_ENTRY(roll)
    REFL_ENTRY(version)
REFL_END()

//! признаки автоматического перехода между фазами полета
struct settings_t
{
    settings_t()
    {
        for(size_t i = 0; i < fms::T_SIZE; ++i)
            auto_transition[i] = true;
    }

    //! автоматический переход между фазами полета
    bool auto_transition[fms::T_SIZE];
};


REFL_STRUCT(settings_t)
    REFL_ENTRY_NAMED(auto_transition[fms::T_START], "auto start")
    REFL_ENTRY_NAMED(auto_transition[fms::T_TAKEOFF], "auto takeoff")
    REFL_ENTRY_NAMED(auto_transition[fms::T_CLIMB], "auto climb")
    REFL_ENTRY_NAMED(auto_transition[fms::T_DESCENT], "auto descent")
    REFL_ENTRY_NAMED(auto_transition[fms::T_APPROACH], "auto approach")
    REFL_ENTRY_NAMED(auto_transition[fms::T_LANDING], "auto landing")
REFL_END()

//! интерфейс, получение основной информации о проводимом ВС
struct info
{
    virtual ~info(){}

    virtual state_t const& get_state()  const = 0;
    virtual double get_mass() const  = 0;
    virtual double get_payload_mass() const  = 0;
    virtual fms::manual_controls_t const& get_controls()  const = 0;
    virtual fms::instrument_ptr           get_instrument() const  = 0;
    virtual fms::instrument_ptr           get_next_instrument() const = 0;
    virtual fms::plan_t const&            get_plan() const  = 0;

    virtual double length_fwd() const = 0;
    virtual double length_bwd() const = 0;

    virtual optional<ada::data_t> const& fsettings() const = 0;

    virtual fms::procedure_model_ptr procedure_model() const = 0;
    virtual fms::operation_model_ptr operation_model() const = 0;

    virtual point_3 ground_velocity()  const = 0;
    virtual double max_fuel() const = 0;
    virtual ani::airport_info_ptr closest_airport() const = 0;
    virtual optional<double> get_desired_course() const = 0; // information only, not state
    virtual optional<double> get_desired_height() const = 0;
    virtual optional<double> get_desired_CAS() const = 0;
    virtual optional<double> get_desired_ROCD() const = 0;

    DECLARE_EVENT(state_changed, ()) ;
    DECLARE_EVENT(fms_changed, ()) ;
    DECLARE_EVENT(plan_changed, ()) ;
};

//! интерфейс, установка некоторых свойств ВС и команды на выполнение некоторых "маневров"
struct control : info
{
    virtual ~control(){}

    virtual void set_state( state_t const& st ) = 0;
    virtual void set_controls( fms::manual_controls_t const& ctrl ) = 0;
    virtual void set_plan    (fms::plan_t const& plan) = 0;

    virtual void transition(fms::transition_t t) = 0;
    virtual void fuel_discharge(double part) = 0;

    virtual void set_aircraft_kind(std::string const &kind) = 0 ;
    virtual void set_payload(double val) = 0;
    
    virtual void instrument_direction(double course) = 0;
    virtual void instrument_turn     (double course, optional<bool> ccw, optional<double> roll) = 0;
    virtual bool instrument_point    (ani::point_pos const& pnt) = 0;
    virtual void instrument_lock     (string const& airport, ani::runway_id runway) = 0;
    virtual void instrument_approach (string const& airport, ani::runway_id runway) = 0;
    virtual void instrument_go_around(string const& airport, ani::runway_id runway) = 0;
    virtual void instrument_star     (ani::sidstar_id id) = 0;
    virtual void instrument_holding  (ani::point_id id, fms::holding::config_t const &cfg) = 0;
    virtual void instrument_cancel_next() = 0;
    virtual void instrument_fpl_offset(double value, optional<geo_point_2> start, optional<geo_point_2> end) = 0;
    virtual void instrument_point_fly_over(bool fly_over) = 0;
    virtual void instrument_point_height_mode(fms::instruments::point::height_mode_t hm) = 0;
    virtual void set_desired_height (optional<double> height) = 0;
    virtual void set_desired_CAS    (optional<double> CAS) = 0;
    virtual void set_desired_ROCD   (optional<double> ROCD) = 0;
};


inline state_t::state_t()
    : pitch(0)
    , roll(0)
    , version(0)
{}

inline state_t::state_t(fms::pilot_state_t const& ps, double pitch, double roll, uint32_t version)
    : fms::pilot_state_t(ps)
    , pitch(pitch)
    , roll(roll)
    , version(version)
{}

inline cpr state_t::orien() const
{
    return cpr(dyn_state.course, pitch, roll);
}

}

}