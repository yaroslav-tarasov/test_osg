#pragma once
#include "cpp_utils/polymorph_ptr.h"

namespace kernel
{

struct object_data;

typedef polymorph_ptr<object_data>          object_data_ptr;
typedef boost::weak_ptr<object_data>        object_data_wptr;

typedef std::vector<object_data_ptr>     object_data_vector;

} // kernel

