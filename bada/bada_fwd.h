#pragma once

#include "common/dyn_lib.h"
#include "cpp_utils/polymorph_ptr.h"

namespace bada
{

struct air_data;
struct glb_data;
struct synonim_data;
struct proxy;

typedef polymorph_ptr<air_data>       air_data_ptr;
typedef polymorph_ptr<glb_data>       glb_data_ptr;
typedef polymorph_ptr<synonim_data>   synonim_data_ptr;
typedef polymorph_ptr<proxy>          proxy_ptr;

}

#ifdef BADA_LIB
# define BADA_API __HELPER_DL_EXPORT
#else
# define BADA_API __HELPER_DL_IMPORT
#endif
