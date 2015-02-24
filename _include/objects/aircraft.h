#pragma once

#include "aircraft_fwd.h"
#include "fpl_fwd.h"
#include "nodes_management_fwd.h"

#include "fms/fms.h"
#include "atc/atc.h"

namespace aircraft
{

//! настройки ответчика
struct responder_settings
{
    responder_settings()
        : code(atc::RC_2000  )
        , mode(atc::RM_A_MODE)
        , type(atc::ST_ICAO  )
        , flags(0)
    {
    }

    uint32_t code;
    uint32_t mode;
    uint32_t type;
    uint32_t flags;
};

//! сравнение ответчиков
inline bool operator !=(responder_settings const &lhs, responder_settings const &rhs)
{
    return lhs.code != rhs.code || lhs.mode != rhs.mode || lhs.type != rhs.type || lhs.flags != rhs.flags;
}

//! настройки ВС
struct settings_t
{
    settings_t()
        : kind          ("B744")
        , turbulence    (0)
        , equipment     (atc::OE_W)
        , payload       (1.)
        , fuelload      (1.)
    {}

    std::string kind;
    std::string company_name;
    std::string airport;
    std::string parking;

    responder_settings responder;
    unsigned turbulence;

    unsigned equipment;

    double payload;  // passengers and luggage load
    double fuelload; // 
    
    std::string responder_str(bool k_code = false) const
    {
        if (atc::RM_OFF == responder.mode)
            return "A0000" ;

        std::string prefix ;

        if (k_code)
            prefix = "K" ;

        std::string suffix ;
        if ((atc::RC_SPI & responder.flags) == atc::RC_SPI)
            suffix = "SPI" ;
        else if ((atc::RC_EMG & responder.flags) == atc::RC_EMG)
            suffix = "EMG" ;
        else if ((atc::RC_2000 & responder.flags) == atc::RC_2000)
            suffix = "2000" ;
        else if ((atc::RC_7500 & responder.flags) == atc::RC_7500)
            suffix = "7500" ;
        else if ((atc::RC_7600 & responder.flags) == atc::RC_7600)
            suffix = "7600" ;
        else if ((atc::RC_7700 & responder.flags) == atc::RC_7700)
            suffix = "7700" ;
        else
            suffix = str(cpp_utils::inplace_format("%d") % responder.code) ;

        return prefix + suffix ;
    }
};

//! некие типы эшелонов(?)
enum atc_level_kind
{
    lk_cfl = 0, //! cleared flight level
    lk_rfl,     //! requested flight level
    lk_xfl,     //! exit flight level
    lk_tfl,     //! transfer flight level
    lk_afl,     //! actual flight level
    lk_count    
};

//! структура содержащая таблицу эшелонов для всех типов эшелонов и 
struct atc_controls_t
{
    atc_controls_t()
    {
        levels.resize(lk_count);
    }

    std::vector<optional<double>> levels;

/*    optional< std::pair<double, unsigned> > course;*/
    optional< std::pair<double, unsigned> > CAS;    //! Calibrated airspeed, Приборная скорость
    optional< std::pair<double, unsigned> > ROCD;   //! Rate of Climb or Descent, скорость подъема или снижения (вертикальная)

     optional<double> course;
//     optional<double> CAS;
//     optional<double> ROCD;

    REFL_INNER(atc_controls_t)
        REFL_ENTRY(levels)
        REFL_ENTRY(course)
        REFL_ENTRY(CAS)
        REFL_ENTRY(ROCD)
    REFL_END()
};

typedef fms::manual_controls_t ipo_controls_t;

REFL_STRUCT(responder_settings)
    REFL_NUM (code, 0, 99999, 1)
    REFL_ENUM(mode, ("OFF", "A", "C", "S", NULL))
    REFL_ENUM(type, ("ICAO", "USSR", NULL))
    REFL_SER (flags)
REFL_END()

REFL_STRUCT(settings_t)
    REFL_ENTRY(kind)
    REFL_ENTRY(company_name)
    REFL_ENTRY(airport)
    REFL_ENTRY(parking)

    REFL_ENTRY(payload)
    REFL_ENTRY(fuelload)

    REFL_ENTRY(responder)
    REFL_FLAG_ENUM(equipment, ("W (RVSM)", NULL))
    REFL_SER(turbulence)
REFL_END()

//! особые случаи (повреждения) ВС
enum malfunction_kind_t
{
    MF_CHASSIS_FRONT = 0,
    MF_CHASSIS_REAR_LEFT,
    MF_CHASSIS_REAR_RIGHT,
    MF_FIRE_ON_BOARD,
    MF_SMOKE_ON_BOARD,
    MF_FUEL_LEAKING,
    MF_ONE_ENGINE,
    MF_ALL_ENGINES,
    
    MF_SIZE
};

//! интерфейс, получение предсказания траектории
struct tp_provider
{
    virtual fms::model_traj_ptr  prediction() = 0;
};

//! интерфейс, получатель информации о ВС
struct fms_container
{
    virtual ~fms_container(){}
    virtual aircraft_fms::info_ptr get_fms() const = 0;
};

//! интерфейс, информация о ВС
struct info
{
    virtual ~info(){}

    virtual geo_point_3 const&  pos     () const = 0;
    virtual point_3             dpos    () const = 0;
    virtual cpr                 orien   () const = 0;
    virtual settings_t const &  settings() const = 0;
    //virtual fpl::info_ptr       get_fpl () const = 0;
    virtual bool                has_assigned_fpl() const = 0;

    virtual transform_4 const&  tow_point_transform() const = 0;

    virtual nodes_management::node_info_ptr root() const = 0;
    virtual bool malfunction(malfunction_kind_t kind) const = 0;

// FIXME not relized yet 
    //virtual tp_provider_ptr  get_tp_provider(double duration_sec) = 0;

    //virtual aircraft_fms::info_ptr get_fms() const = 0;


    //virtual atc_controls_t const& get_atc_controls() const = 0;
    //virtual ipo_controls_t const& get_ipo_controls() const = 0;

    //virtual aircraft_gui::control_ptr get_gui() const = 0;

    virtual optional<double> get_prediction_length() const = 0;
    virtual optional<double> get_proc_length() const = 0;

    DECLARE_EVENT(assigned_fpl_changed, (fpl::info_ptr));
    DECLARE_EVENT(responder_changed, (fpl::info*));
};

//! интерфейс, управление ВС (общее?)
struct control
{
    virtual ~control(){}

    virtual void assign_fpl  (fpl::info_ptr fpl_obj) = 0;
    virtual void unassign_fpl() = 0;
//
    virtual void set_kind(std::string const& kind) = 0;
    virtual void set_turbulence(unsigned turb) = 0;
//
//    virtual void set_atc_controls(atc_controls_t const& controls) = 0 ;
//    virtual void set_ipo_controls(ipo_controls_t const& controls) = 0 ;
};

//! интерфейс, управление ВС (со стороны пилота-оператора ?)
struct aircraft_ipo_control
{
    virtual ~aircraft_ipo_control(){}

    virtual void set_malfunction(malfunction_kind_t kind, bool enabled) = 0;

    virtual void set_cmd_go_around(uint32_t cmd_id)                                       = 0;
    virtual void set_cmd_holding  (uint32_t cmd_id, fms::holding_t const &holding)        = 0;
    virtual void set_cmd_course   (uint32_t cmd_id, fms::course_modifier_t const &course) = 0;
    virtual void cancel_cmd       (uint32_t cmd_id)                                       = 0;

    virtual void set_responder_mode(atc::responder_mode mode) = 0;
    virtual void set_responder_type(atc::squawk_type stype) = 0;
    virtual void set_responder_code(unsigned code) = 0;
    virtual void set_responder_flag(unsigned flag, bool enable) = 0;

    virtual void restore_responder_code() = 0;
};

//! интерфейс, управление ВС (со стороны диспетчера ?)
struct aircraft_atc_control
{
    virtual ~aircraft_atc_control(){}

    virtual void change_speed_display(boost::optional<double> value) = 0 ;
    virtual void change_dist_display(boost::optional<double> value)  = 0 ;
    virtual void set_new_callsign(boost::optional<std::string>) = 0 ;
    virtual void activate () = 0 ;
};

} // end of aircraft

