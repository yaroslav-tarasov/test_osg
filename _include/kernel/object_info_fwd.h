#pragma once
#include "cpp_utils/polymorph_ptr.h"

namespace kernel
{

struct object_info;
struct object_class;

typedef polymorph_ptr<object_info>          object_info_ptr;
typedef boost::weak_ptr<object_info>        object_info_wptr;
typedef polymorph_ptr<object_class>         object_class_ptr;

typedef std::vector<object_class_ptr>    object_class_vector;
typedef std::vector<object_info_ptr>     object_info_vector;

} // kernel

