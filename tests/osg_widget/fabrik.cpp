#include "stdafx.h"
#include "widget.h"
#include "OSGWidget.h"

namespace visual
{

    // export function implementation
    visual_widget_ptr create_widget( visual_type v )
    {
        if(v == CUSTOM_GL_WIDGET)
            return boost::make_shared<Widget>();
        else
            return boost::make_shared<OSGWidget>();
    }


}