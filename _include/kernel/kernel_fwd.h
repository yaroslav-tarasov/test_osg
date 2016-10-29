#pragma once

#include "object_info_fwd.h"
#include "systems_fwd.h"
#include "bin_data/io_streams_fwd.h"

namespace kernel
{
struct msg_service;

typedef boost::optional<binary::input_stream &>     istream_opt;
typedef boost::optional<binary::output_stream&>     ostream_opt;
typedef binary::output_stream&                      ostream_ref;

} // kernel
