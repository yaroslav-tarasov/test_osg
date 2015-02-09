#pragma once

#include "aircraft_fwd.h"

#include "phys_sys_fwd.h"
#include "position.h"


//! самолет

namespace aircraft
{

//! получение имени модели по имени типа ВС (да, вот следствие того что опираются только на БАДУ а не на свою базу)
inline std::string get_model(std::string const& kind)
{
    // TODO : move to external config
    if (kind == "A321")
        return "a_321";
    else if (kind == "A333")
        return "a_330_300";
    else if (kind == "A319")
        return "a_319";
    else if (kind == "B737")
        return "b_737";
    else if (kind == "B744")
        return "b_747_400";
    else if (kind == "B763")
        return "b_767_300";
    else if (kind == "Mi8")
        return "mi_8";
    else if (kind == "C525")
        return "cessna_525";
    else
        return "checker";
}

//! получение имени текстуры по имен типа ВС и авиакомпании (да, у нас тоже нет промежуточной сущности уровня "самолет" в БД района)
inline std::string get_texture(std::string const& kind, std::string const& company_name)
{
    // TODO : move to external config
    if (kind == "B737")
    {
        if (company_name == "UTA")
            return "utair" ;
        else if (company_name == "GZP")
            return "gazprom";
        else if (company_name == "AEW")
            return "aerosvit";
        else if (company_name == "TSO")
            return "transaero";
        else if (company_name == "SYL")
            return "yakutia";
        else //if (company_name == "DNV")
            return "kuban";
    }
    else if (kind == "A319")
    {
        if (company_name == "AFR")
            return "airfrance" ;
        else //if (company_name == "SDM")
            return "rossia";
    }
    else if (kind == "A321")
    {
        if (company_name == "AFL")
            return "aeroflot" ;
        else //if (company_name == "KLM")
            return "klm";
    }
    else if (kind == "A333")
    {
        if (company_name == "AFL")
            return "aeroflot" ;
        else //if (company_name == "DAL")
            return "delta";
    }
    else if (kind == "B744")
    {
        if (company_name == "TSO")
            return "transaero";
        else
            return "cargolux";
    }
    else if (kind == "B763")
    {
        if (company_name == "AFL")
            return "aeroflot" ;
        else // KOR
            return "koreanair";
    }

    return "";
}

//! интерфейс информации о модели
struct model_info
{
    virtual ~model_info() {}

    //! для bullet  - твердое тело самолета
    virtual phys::rigid_body_ptr get_rigid_body() const = 0;
    //! прицеплен ли буксир
    virtual bool tow_attached() const = 0;
    //! координата буксира
    virtual cg::point_3     tow_offset() const = 0;
    //! физическая позиция, включающая координаты и ориентацию в пространстве
    virtual geo_position get_phys_pos() const = 0;
};

//! интерфейс управления моделью
struct model_control
{
    virtual ~model_control() {}

    //! прицепить буксир
    virtual void set_tow_attached(optional<uint32_t> attached, boost::function<void()> tow_invalid_callback) = 0;
    //! ??? направить по курсу???
    virtual void set_steer( double steer ) = 0;
};


///  From view
//! интерфейс, информация о ВС
struct info
{
    virtual ~info(){}

    //virtual geo_point_3 const&  pos     () const = 0;
    //virtual point_3             dpos    () const = 0;
    //virtual cpr                 orien   () const = 0;
    //virtual settings_t const &  settings() const = 0;
    //virtual fpl::info_ptr       get_fpl () const = 0;
    //virtual bool                has_assigned_fpl() const = 0;

    //virtual transform_4 const&  tow_point_transform() const = 0;

    //virtual nodes_management::node_info_ptr root() const = 0;
    //virtual bool malfunction(malfunction_kind_t kind) const = 0;

    //virtual tp_provider_ptr  get_tp_provider(double duration_sec) = 0;

    //virtual aircraft_fms::info_ptr get_fms() const = 0;


    //virtual atc_controls_t const& get_atc_controls() const = 0;
    //virtual ipo_controls_t const& get_ipo_controls() const = 0;

    //virtual aircraft_gui::control_ptr get_gui() const = 0;

    //virtual optional<double> get_prediction_length() const = 0;
    //virtual optional<double> get_proc_length() const = 0;

    //DECLARE_EVENT(assigned_fpl_changed, (fpl::info_ptr));
    //DECLARE_EVENT(responder_changed, (fpl::info*));
};

//! интерфейс, управление ВС (общее?)
struct control
{
    virtual ~control(){}

    //virtual void assign_fpl  (fpl::info_ptr fpl_obj) = 0;
    //virtual void unassign_fpl() = 0;

    //virtual void set_kind(std::string const& kind) = 0;
    //virtual void set_turbulence(unsigned turb) = 0;

    //virtual void set_atc_controls(atc_controls_t const& controls) = 0 ;
    //virtual void set_ipo_controls(ipo_controls_t const& controls) = 0 ;
};


} // end of aircraft
