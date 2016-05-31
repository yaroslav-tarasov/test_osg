#pragma once
#undef SYSTEMS_API
#include "aircraft/aircraft_common.h"

#include "kernel/kernel.h"

#include "kernel/object_class.h"
#include "kernel/object_info.h"
#include "kernel/object_data.h"

#include "network/msg_base.h"


using namespace kernel;
using namespace binary;
using network::gen_msg;
#include "kernel/systems/systems_base.h"
#include "kernel/objects/base_view.h"

#include "kernel/systems/vis_system.h"
#include "kernel/systems/ctrl_system.h"

