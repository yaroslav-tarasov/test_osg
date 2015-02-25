#pragma once

#include "aircraft_fwd.h"
#include "objects/aircraft_fms.h"

//! нечто относ€щеес€ к системе сообщений, св€занных с математической моделью ¬—

namespace aircraft
{

namespace aircraft_fms
{
//! интерфейс информации о модели?? (предсказание следующей точки?)
struct model_info
{                
    virtual ~model_info() {}

    virtual geo_point_3 prediction(double dt) const = 0;
};
//! интерфейс управлени€ моделью ???
struct model_control
{
    virtual ~model_control(){}

    virtual void reset_pos(geo_point_3 const& pos, double c) = 0;
    virtual void activate() = 0;
};


namespace msg 
{
    //! некие свойства ¬—, доступ к которым осуществл€етс€ через сообщени€??
    enum id
    {
        afm_state = 0,
        afm_kind,
        afm_payload,
        afm_transition,
        afm_fuel_discharge,
        afm_settings,
        afm_controls,
        afm_plan,
        afm_instrument
    };

    //! сообщение"состо€ние" - вместо gen_msg определ€ем €вно; в чем преимущества/недостатки? 
    struct state_msg
        : network::msg_id<afm_state>
    {               
        state_msg();
        explicit state_msg(state_t const& state);
        operator state_t() const;

        geo_point_2 pos;
        float       height;
        cprf        orien;
        float       TAS;
        float       fuel_mass;
        unsigned char cfg;
        unsigned char alt_state;
        uint32_t version;
    };
    
    //! все остальные сообщени€ - генерируютс€ с использованием универсального шаблона
    typedef network::gen_msg<afm_kind           , string>                       kind_msg;
    typedef network::gen_msg<afm_payload        , double>                       payload_msg;
    typedef network::gen_msg<afm_transition     , uint32_t>                     transition_msg;
    typedef network::gen_msg<afm_fuel_discharge , double>                       fuel_discharge_msg;
    typedef network::gen_msg<afm_settings       , settings_t>                   settings_msg;
    typedef network::gen_msg<afm_controls       , fms::manual_controls_t>       controls_msg;
    typedef network::gen_msg<afm_plan           , std::vector<binary::bytes_t>> plan_msg;

    REFL_STRUCT(state_msg)
        REFL_ENTRY(pos)
        REFL_ENTRY(height)
        REFL_ENTRY(orien)
        REFL_ENTRY(TAS)
        REFL_ENTRY(fuel_mass)
        REFL_ENTRY(cfg)
        REFL_ENTRY(alt_state)
        REFL_ENTRY(version)
    REFL_END   ()


    inline state_msg::state_msg()
    {
    }

    inline state_msg::state_msg(state_t const& state)
        : pos(state.dyn_state.pos)
        , height((float)state.dyn_state.pos.height)
        , orien(state.orien())
        , TAS((float)state.dyn_state.TAS)
        , cfg((unsigned char)state.dyn_state.cfg)
        , fuel_mass((float)state.dyn_state.fuel_mass)
        , alt_state(state.alt_state)
        , version(state.version)
    {
    }

    inline state_msg::operator state_t() const
    {
        fms::pilot_state_t pilot(fms::state_t(geo_point_3(pos, height), orien.course, fuel_mass, TAS, (fms::air_config_t)cfg), (fms::alt_state_t)alt_state);

        return state_t(pilot, orien.pitch, orien.roll, version);
    }

} // msg


} // end of aircraft_fms
} // end of aircraft