#pragma once

#include "cpp_utils/polymorph_ptr.h"

namespace camera_object
{
	struct info;
	struct control;

	typedef polymorph_ptr<info>    info_ptr;
	typedef boost::weak_ptr<info>  info_wptr;
	typedef polymorph_ptr<control> control_ptr;    
	
	struct model_info;
    struct model_control;
    struct model_ext_control;

    typedef polymorph_ptr<model_info>        model_info_ptr;
    typedef polymorph_ptr<model_control>     model_control_ptr;
    typedef polymorph_ptr<model_ext_control> model_ext_control_ptr;
    
}


