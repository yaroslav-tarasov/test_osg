#pragma once

#include "common/event.h"

namespace aircraft
{

enum atc_state_t
{
    AS_NOT_CONCERNED = 0,       // Не относящееся             (DEACT)
    AS_NOT_CONCERNED_ABI,       // Поступил oldi с сообщением (ABI)
    AS_ACTIVATED,               // Активизированное           (ACT)    (рфс)

    AS_COUNT
};

struct atc_info
{
    virtual ~atc_info() {}

    virtual atc_state_t get_atc_state() const = 0 ;
    virtual void        set_atc_state(atc_state_t const &astate) = 0 ;

    virtual bool        ssr_synchronized() const = 0 ;
    virtual boost::optional<std::string> get_new_callsign() const = 0;
    virtual std::vector< std::pair<double, std::string> > get_oldi_history() const = 0;


    DECLARE_EVENT(atc_state_changed, (atc_state_t const &)) ;
};

} // end of aircraft
