#pragma once

#include "cpp_utils/polymorph_ptr.h"

#include "common/test_msgs.h"

namespace aircraft_reg
{

struct info
{
    virtual ~info() {}
};

struct control
{
    virtual void inject_msg(uint32_t obj_id, net_layer::msg::run const& msg) = 0;
    virtual void inject_msg(net_layer::msg::container_msg const& msg)=0;
    virtual ~control() {}
};

typedef polymorph_ptr<info> info_ptr;
typedef polymorph_ptr<control> control_ptr;

}