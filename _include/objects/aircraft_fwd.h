#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace aircraft
{
    struct tp_provider;
    struct info;
    struct control;
    struct fms_container;
    struct aircraft_ipo_control;
    struct aircraft_atc_control;

    typedef boost::shared_ptr<tp_provider>      tp_provider_ptr;
    typedef polymorph_ptr<info>                 info_ptr;
    typedef boost::weak_ptr<info>               info_wptr;
    typedef polymorph_ptr<control>              control_ptr;
    typedef polymorph_ptr<fms_container>        fms_container_ptr;
    typedef polymorph_ptr<aircraft_ipo_control> aircraft_ipo_control_ptr;
    typedef polymorph_ptr<aircraft_atc_control> aircraft_atc_control_ptr;

    namespace aircraft_fms
    {
        struct info;
        struct control;

        typedef polymorph_ptr<info>    info_ptr;
        typedef polymorph_ptr<control> control_ptr;
    }

    namespace aircraft_gui
    {
        struct control;
        struct info;
        typedef polymorph_ptr<control>  control_ptr;
        typedef polymorph_ptr<info>     info_ptr;
    } 
}