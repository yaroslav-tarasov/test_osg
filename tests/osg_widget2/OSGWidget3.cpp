#include "OSGWidget3.h"

#include <cassert>

#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QBoxLayout>

#include <osgViewer/CompositeViewer>

#include "av/Visual.h"
#include <osgQt/GraphicsWindowQt>

#ifndef _DEBUG
#pragma comment(lib, "osgQt.lib")
#else 
#pragma comment(lib, "osgQtd.lib")
#endif

namespace visual
{

    osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false )
    {
        osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->windowName = name;
        traits->windowDecoration = windowDecoration;
        traits->x = x;
        traits->y = y;
        traits->width = w;
        traits->height = h;
        traits->doubleBuffer = true;
        traits->alpha = 8;//ds->getMinimumNumAlphaBits();
        traits->stencil = ds->getMinimumNumStencilBits();
        traits->sampleBuffers = ds->getMultiSamples();
        traits->samples = 8;//ds->getNumMultiSamples();

        return new osgQt::GraphicsWindowQt(traits.get());
    }

    OSGWidget3::OSGWidget3()
    {
        setLayout(new QBoxLayout(QBoxLayout::LeftToRight));
        layout()->setContentsMargins(0, 0, 0, 0);
        this->setFocusPolicy( Qt::StrongFocus );
        this->setMouseTracking( true );

        initialize();
    }
                                                    
    void OSGWidget3::initialize()
    {
        osgQt::GraphicsWindowQt* gw = createGraphicsWindow( x(), y(), width(), height() , "VISUAL" );

        engine_ = CreateVisual();
        
        osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(engine_->GetViewer());
        osgViewer::ViewerBase::Cameras cams_;
        v->getCameras(cams_,false);
        cams_[0]->setGraphicsContext( gw );
        cams_[0]->setViewport(new osg::Viewport(0, 0, width(), height() ));
        
        // gw->setTouchEventsEnabled( true );
        child_ = gw->getGLWidget();  
        layout()->addWidget(child_);
        
    }

    void OSGWidget3::createScene( )
    {
         initialize();
    }

    void OSGWidget3::endSceneCreation( )
    {
    
    }

    void  OSGWidget3::redraw()
    {
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
        force_log fl;
        LOG_ODS_MSG( "redraw()" << _hr_timer.get_delta() << "\n" );
    }


    bool OSGWidget3::event(QEvent *event)
    {
        switch (event->type()) {
        case QEvent::UpdateRequest:
           {
                //force_log fl;
                LOG_ODS_MSG( "redraw()" << _hr_timer.get_delta() << "\n" );
                engine_->Render();
           }
            return true;
        default:
            return QWidget::event(event);
        }
    }
}