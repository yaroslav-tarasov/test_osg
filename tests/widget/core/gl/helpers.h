#pragma once
#include "../context/base_context.h"

namespace core
{

static const unsigned int primitive_restart_index = -1;

struct ContextLocker
{
    ContextLocker( context_ptr cont_ptr, CoreWidget const * widget = nullptr )
        : context_(cont_ptr)
    {
        if(context_)
            context_->makeCurrent(widget);
    }
    ~ContextLocker()
    {
        if(context_)
            context_->doneCurrent();
    }

private:

    context_ptr context_;
};

}

#define MAKE_CONTEXT_CURRENT \
    core::ContextLocker const lock(context_);

// #define ASSERT_CONTEXT_CURRENT \
//     if (!core::create_context()->isCurrent()) {      \
//         LogError("OpenGL : context is not locked"); \
//     }

#define ASSERT_CONTEXT_CURRENT VerifyMsg(core::create_context()->isCurrent(), "GLChart: context is not locked!");
