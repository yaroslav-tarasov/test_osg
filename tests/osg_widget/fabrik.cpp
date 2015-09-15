#include "stdafx.h"
#include "widget.h"
#include "OSGWidget.h"
#include "OSGWidget2.h"
#include "OSGWidget3.h"
#include "TriangleWindow.h"

namespace visual
{

    // export function implementation
    visual_widget_ptr VISUAL_API create_widget( visual_type v )
    {
        if(v == CUSTOM_GL_WIDGET)
            return boost::make_shared<Widget>();
        else  
        if(v == CUSTOM_GL_WIDGET3)
            return boost::make_shared<OSGWidget3>();
        else
        if(v == CUSTOM_GL_WIDGET1)
        {
            auto tw = boost::make_shared<TriangleWindow>();

            QSurfaceFormat format;
            format.setAlphaBufferSize(8);
            format.setBlueBufferSize(8);
            format.setGreenBufferSize(8);
            format.setRedBufferSize(8);
            format.setDepthBufferSize(24);
            format.setStencilBufferSize(8);
            format.setSamples(8);
            format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
            format.setMajorVersion(1);  
            format.setMinorVersion(5);
            format.setProfile( QSurfaceFormat::CoreProfile );

            tw->setFormat(format);
            tw->show();
            // tw->setAnimating(true);

            return tw;
        }
        else
        if(v == CUSTOM_GL_WIDGET2)
        {
            auto tw = boost::make_shared<OSGWidget2>();

            QSurfaceFormat format;
            format.setAlphaBufferSize(8);
            format.setBlueBufferSize(8);
            format.setGreenBufferSize(8);
            format.setRedBufferSize(8);
            format.setDepthBufferSize(24);
            format.setStencilBufferSize(8);
            format.setSamples(8);
            format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
            format.setProfile( QSurfaceFormat::CoreProfile );

            tw->setFormat(format);
            tw->show();
            // tw->setAnimating(true);

            return tw;
        }
        else
            return boost::make_shared<OSGWidget>();
    }


}