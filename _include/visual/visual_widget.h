#pragma once

#include <boost/smart_ptr.hpp>

#include "visual/visual_api.h" 

namespace visual
{
    enum visual_type {QOGL_WIDGET, CUSTOM_GL_WIDGET, CUSTOM_GL_WIDGET1, CUSTOM_GL_WIDGET2, CUSTOM_GL_WIDGET3,CUSTOM_GL_WIDGET4 };

    struct visual_widget 
    {
        virtual void redraw() = 0;
        virtual void createScene() =0;
        virtual void endSceneCreation() =0;
    };

    typedef boost::shared_ptr<visual_widget> visual_widget_ptr;

    VISUAL_API visual_widget_ptr create_widget(visual_type v);

} // end of namespace visual
