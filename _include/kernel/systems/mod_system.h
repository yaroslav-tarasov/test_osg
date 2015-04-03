#pragma once
#include "common/event.h"

namespace kernel
{

struct model_system
{
    virtual double calc_step() const = 0;
    virtual ~model_system(){}
};

struct model_presentation
{
    virtual ~model_presentation(){}
};

} // kernel
