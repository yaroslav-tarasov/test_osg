#pragma once

#include "aircraft_fwd.h"
#include "objects/aircraft.h"
#include "objects/aircraft_fms.h"

#include "phys/phys_sys_fwd.h"
#include "atc/position.h"


namespace aircraft
{

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
    else if (kind == "Mi8" || kind == "Ã»-8")
        return "mi_8";
    else if (kind == "C525")
        return "cessna_525";
    else if (kind == "AN26" || kind == "¿Õ-26")
        return "an_26";
    else if (kind == "SB20")
        return "sb20"; 
    else if (kind == "KA27" || kind == " ¿-27")
        return "ka27";
    else if (kind == "KA50"|| kind == " ¿-50") 
        return "ka_50";
    else if (kind == "KA52" || kind == " ¿-52" )
        return "ka_52";
    else if (kind == "L39" || kind == "À-39")
        return "l_39";
	else if (kind == "AN140" || kind == "¿Õ-140")
		return "an_140";
    else if (kind == "IL76" || kind == "»À-76")
        return "il_76";
    else if (kind == "TU154" || kind == "“”-154")
        return "tu_154";  
    else if (kind == "TU134" || kind == "“”-134")
        return "tu_134";    
    else if (kind == "SU25" || kind == "—”-25")
        return "su_25tm";
    else if (kind == "SU27" || kind == "—”-27")
        return "su_27";
    else if (kind == "MIG29" || kind == "Ã»√-29")
        return "mig_29";
	else if (kind == "MIG29K" || kind == "Ã»√-29 ")
		return "mig_29k";
    else
        return "checker";
}

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

struct model_info
{
    virtual ~model_info() {}

    virtual phys::rigid_body_ptr get_rigid_body() const = 0;
    virtual bool                 tow_attached  () const = 0;
    virtual cg::point_3          tow_offset    () const = 0;
    virtual geo_position         get_phys_pos  () const = 0;
    virtual double               rotors_angular_speed () const = 0;
};

struct model_control
{
    virtual ~model_control() {}

    virtual void                 set_tow_attached         (optional<uint32_t> attached, boost::function<void()> tow_invalid_callback) = 0;
    virtual void                 set_steer                ( double steer ) = 0;
    virtual void                 set_brake                ( double brake ) = 0;
    virtual void                 set_rotors_angular_speed ( double val ) = 0;
};

struct model_ext_control
{
	virtual ~model_ext_control() {}
	virtual void                 set_desired   (double time,const cg::point_3& pos, const cg::quaternion& q, const double speed, fms::trajectory::air_config_t ac =  fms::trajectory::CFG_SIZE )     = 0;
	virtual void                 set_ext_wind  (double speed, double azimuth) =0;
};

} // end of aircraft
