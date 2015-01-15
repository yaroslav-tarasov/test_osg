#pragma once

#include "bada_fwd.h"
#include "bada_air_data.h"
#include "bada_glb_data.h"

//#include "fms/fms.h"

#define BADA_API

namespace bada
{

struct proxy
{
    virtual ~proxy(){}

    virtual std::vector<std::string> get_aircrafts() const = 0 ;
    virtual air_data_ptr     get_aircraft_data(std::string const& synonim) const = 0 ;
    virtual glb_data_ptr     get_global_data() const = 0 ;
    virtual synonim_data_ptr get_synonim_data(std::string const& air_kind) const = 0 ;
};

BADA_API proxy_ptr create_proxy(std::string const& bada_path) ;

} // end of bada
