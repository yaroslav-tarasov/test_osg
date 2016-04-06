#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace aircraft
{
    struct int_control;   // FIXME все внутрь ибо нефиг
    typedef polymorph_ptr<int_control>          int_control_ptr;
}

namespace aircraft
{

    struct ani_info;
    struct model_info;
    struct model_control;
	struct model_ext_control;
    struct atc_info;
    struct chart_info;
    struct chart_control;

    typedef polymorph_ptr<ani_info>         ani_info_ptr;
    typedef polymorph_ptr<model_info>       model_info_ptr;
    typedef polymorph_ptr<model_control>    model_control_ptr;
	typedef polymorph_ptr<model_ext_control> model_ext_control_ptr;
    typedef polymorph_ptr<atc_info>         atc_info_ptr;
    typedef polymorph_ptr<chart_info>       chart_info_ptr;
    typedef polymorph_ptr<chart_control>    chart_control_ptr;

    namespace aircraft_fms
    {

        struct info;
        struct model_control;
        struct model_info;

        typedef polymorph_ptr<info>           info_ptr;
        typedef polymorph_ptr<model_control>  model_control_ptr;
        typedef polymorph_ptr<model_info>     model_info_ptr;

    } // end of aircraft_fms

    namespace aircraft_gui
    {

        struct chart_info;
        struct chart_control;

        typedef polymorph_ptr<chart_info>     chart_info_ptr;
        typedef polymorph_ptr<chart_control>  chart_control_ptr;
    } 

}