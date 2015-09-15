#pragma once

#include <QtCore>

#include "widget/core_widget.h"

#include "context/base_context.h"

namespace core
{
    core::context_ptr create_context();
}

#include "context/context_object.h"

#include "gl/name.h"
#include "gl/uniforms.h"
#include "gl/helpers.h"
