#include "OSGWidget2.h"

#include <cassert>

#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>

#include <osgViewer/CompositeViewer>

#include "av/Visual.h"
#include <osgViewer/api/Win32/GraphicsWindowWin32>

namespace visual
{

    OSGWidget2::OSGWidget2()
    {
    }
                                                    
    void OSGWidget2::initialize()
    {
        // OSG graphics context
        osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits();
        pTraits->inheritedWindowData           = new osgViewer::GraphicsWindowWin32::WindowData((HWND)/*widget_base->*/winId());
        pTraits->alpha                         = 8;
        pTraits->setInheritedWindowPixelFormat = true;
        pTraits->doubleBuffer                  = true;
        pTraits->windowDecoration              = true;
        pTraits->sharedContext                 = NULL;
        pTraits->supportsResize                = true;
        pTraits->vsync                         = true;
        pTraits->depth                         = 24;
        pTraits->blue                          = 8;
        pTraits->red                           = 8;
        pTraits->green                         = 8;
        pTraits->stencil                       = 8;
        pTraits->samples                       = 4;
        pTraits->x = x();
        pTraits->y = y();
        pTraits->width = width();
        pTraits->height = height();

        engine_ = CreateVisual();
        
        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(pTraits.get());
        if (!gc)
        {
            osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
            return;
        }

        osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(engine_->GetViewer());
        osgViewer::ViewerBase::Cameras cams_;
        v->getCameras(cams_,false);
        cams_[0]->setGraphicsContext( gc.get() );
        cams_[0]->setViewport(new osg::Viewport(0, 0, width(), height() ));
    }

    void OSGWidget2::render()
    {
        engine_->Render();
    }

    void OSGWidget2::createScene( )
    {
         initialize();
    }

    void OSGWidget2::endSceneCreation( )
    {
    
    }

    void  OSGWidget2::redraw()
    {
         QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}