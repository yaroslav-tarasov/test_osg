#pragma once

#include "cpp_utils/polymorph_ptr.h"

#include "common/ext_msgs.h"

namespace objects_reg
{

struct info
{
    virtual ~info() {}
};

struct control
{
    typedef function<void(binary::bytes_cref /*bytes*/)> remote_send_f;

    virtual void inject_msg     (const void* data, size_t size) = 0;
	virtual void create_object  (net_layer::msg::create_msg const& msg)=0;
	virtual void destroy_object (net_layer::msg::destroy_msg const& msg)=0;
    virtual void set_sender     (remote_send_f s)=0;
    
    virtual ~control() {}
};

typedef polymorph_ptr<info> info_ptr;
typedef polymorph_ptr<control> control_ptr;

}