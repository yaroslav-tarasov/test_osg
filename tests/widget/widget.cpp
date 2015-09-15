// #include "engine/scene/scene.h"

//@ собственная glchart-реализация виджета, не входит в Синтез (хотя файлы есть)
//@ 

#include "widget.h"

#include "av/Visual.h"
#include <osgViewer/api/Win32/GraphicsWindowWin32>

namespace visual
{

//
// Base widget
//

struct Widget::WidgetPImpl
{
    WidgetPImpl( CoreWidget * widget_base, int x, int y, int width, int height )
        : selectionActive_(false)
        , selectionFinished_(false)
    {
        render_active_ = false;

        context_ = core::create_context();
        
        WId hwnd = WId(context_->createCompatibleWindow(Window(widget_base->winId())));
        widget_base->reattach_to_window(hwnd);


        // OSG graphics context
        osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits();
        pTraits->inheritedWindowData           = new osgViewer::GraphicsWindowWin32::WindowData((HWND)widget_base->winId());
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
        pTraits->x = x;
        pTraits->y = y;
        pTraits->width = width;
        pTraits->height = height;

        //scene_ = new engine::Scene();
        
        //graphicsWindow_ = new osgViewer::GraphicsWindowEmbedded( pTraits/*x, y, width, height*/);
        
        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(pTraits.get());
        if (!gc)
        {
            osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
            return;
        }

        ;
        
        // widget_base->reattach_to_window((WId)dynamic_cast<osgViewer::GraphicsHandleWin32*>(gc.get())->getHWND());
              
        engine_ = CreateVisual();

        osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(engine_->GetViewer());
        osgViewer::ViewerBase::Cameras cams_;
        v->getCameras(cams_,false);
        cams_[0]->setGraphicsContext( gc.get() );
        cams_[0]->setViewport(new osg::Viewport(0, 0, width, height ));

        osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
        manipulator->setAllowThrow( false );

        v->setCameraManipulator( manipulator );
 
    }
    
    osgGA::EventQueue* getEventQueue() const
    {
        osgGA::EventQueue* eventQueue = graphicsWindow_.valid() ?graphicsWindow_->getEventQueue() : nullptr;

        //if( eventQueue )
            return eventQueue;
        //else
        //    throw std::runtime_error( "Unable to obtain valid event queue");
    }
    
    void resizeGL( int x, int y, int width, int height )
    {
        if(getEventQueue()) getEventQueue()->windowResize( x, y, width, height );
        if(graphicsWindow_.valid()) graphicsWindow_->resized( x, y, width, height );

        onResize( width, height );
    }

    void onResize( int width, int height )
    {
        std::vector<osg::Camera*> cameras;
        osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(engine_->GetViewer());
        v->getCameras( cameras );

        // assert( cameras.size() == 2 );

        if(cameras.size()==2)
        { 
            cameras[0]->setViewport( 0, 0, width / 2, height );
            cameras[1]->setViewport( width / 2, 0, width / 2, height );
        }
        else   if(cameras.size()==1)
        {
            cameras[0]->setViewport( 0, 0, width, height );
        }

    }
    
    void onHome()
    {
        osgViewer::ViewerBase::Views views;
        osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(engine_->GetViewer());
        v->getViews( views );

        for( std::size_t i = 0; i < views.size(); i++ )
        {
            osgViewer::View* view = views.at(i);
            view->home();
        }
    }

    void processSelection()
    {

    }

    bool                render_active_;
    IVisual*            engine_;
    core::context_ptr   context_;
    
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded>			    graphicsWindow_;

    QPoint		    selectionStart_;
    QPoint		    selectionEnd_;

    bool			selectionActive_;
    bool			selectionFinished_;
};

// ctor
Widget::Widget()
{
    impl_ = new WidgetPImpl(this,
        this->x(),
        this->y(),
        this->width(),
        this->height());

    this->setFocusPolicy( Qt::StrongFocus );

    this->setMouseTracking( true );
}

// dtor
Widget::~Widget()
{
    delete impl_;
}

//
// visual_widget
//

//scene_view * Widget::view() const
//{
//    return impl_->scene_.get();
//}

void Widget::createScene()
{

}

void Widget::endSceneCreation()
{

}

void Widget::redraw()
{
    QWidget::repaint();
}

//
// Main overloaded events
//

void Widget::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event)

    // protect from cyclic draws
    if (!impl_->render_active_)
    {
        // render in
        impl_->render_active_ = true;

        core::ContextLocker cl(impl_->context_, this);
        // static_cast<engine::Scene *>(impl_->scene_.get())->doPaint();
        impl_->engine_->Update();
        impl_->engine_->Render();
        impl_->context_->swapBuffers(this);
    }
    // render out
    impl_->render_active_ = false;
}

void Widget::resizeEvent( QResizeEvent * event )
{
    // static_cast<engine::Scene *>(impl_->scene_.get())->setViewport(event->size().width(), event->size().height());
    // impl_->resizeGL( this->x(), this->y(), event->size().width(), event->size().height() );
}

void Widget::changeEvent( QEvent * event )
{
    switch (event->type())
    {
    case QEvent::ParentChange:
        LogWarn("Widget::QEvent::ParentChange");
        reattach_to_window(WId(impl_->context_->createCompatibleWindow(Window(winId()))));
        break;
    default:
        break;
    }
}


void Widget::keyPressEvent( QKeyEvent* event )
{
    QString keyString   = event->text();
    const char* keyData = keyString.toLocal8Bit().data();

    if( event->key() == Qt::Key_S )
    {
#ifdef WITH_SELECTION_PROCESSING
        impl_->selectionActive_ = !impl_->selectionActive_;
#endif

        // Further processing is required for the statistics handler here, so we do
        // not return right away.
    }
    else if( event->key() == Qt::Key_H )
    {
        impl_->onHome();
        return;
    }

    if (impl_->getEventQueue()) impl_->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void Widget::keyReleaseEvent( QKeyEvent* event )
{
    QString keyString   = event->text();
    const char* keyData = keyString.toLocal8Bit().data();

    if (impl_->getEventQueue()) impl_->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void Widget::mouseMoveEvent( QMouseEvent* event )
{
    // Note that we have to check the buttons mask in order to see whether the
    // left button has been pressed. A call to `button()` will only result in
    // `Qt::NoButton` for mouse move events.
    if( impl_->selectionActive_ && event->buttons() & Qt::LeftButton )
    {
        impl_->selectionEnd_ = event->pos();

        // Ensures that new paint events are created while the user moves the
        // mouse.
        this->update();
    }
    else
    {
        if (impl_->getEventQueue())
            impl_->getEventQueue()->mouseMotion( static_cast<float>( event->x() ),
            static_cast<float>( event->y() ) );
    }
}

void Widget::mousePressEvent( QMouseEvent* event )
{
    // Selection processing
    if( impl_->selectionActive_ && event->button() == Qt::LeftButton )
    {
        impl_->selectionStart_    = event->pos();
        impl_->selectionEnd_      = impl_->selectionStart_; // Deletes the old selection
        impl_->selectionFinished_ = false;           // As long as this is set, the rectangle will be drawn
    }

    // Normal processing
    else
    {
        // 1 = left mouse button
        // 2 = middle mouse button
        // 3 = right mouse button

        unsigned int button = 0;

        switch( event->button() )
        {
        case Qt::LeftButton:
            button = 1;
            break;

        case Qt::MiddleButton:
            button = 2;
            break;

        case Qt::RightButton:
            button = 3;
            break;

        default:
            break;
        }

        if (impl_->getEventQueue()) 
            impl_->getEventQueue()->mouseButtonPress( static_cast<float>( event->x() ),
            static_cast<float>( event->y() ),
            button );
    }
}

void Widget::mouseReleaseEvent(QMouseEvent* event)
{
    // Selection processing: Store end position and obtain selected objects
    // through polytope intersection.
    if( impl_->selectionActive_ && event->button() == Qt::LeftButton )
    {
        impl_->selectionEnd_      = event->pos();
        impl_->selectionFinished_ = true; // Will force the painter to stop drawing the
        // selection rectangle

        impl_->processSelection();
    }

    // Normal processing
    else
    {
        // 1 = left mouse button
        // 2 = middle mouse button
        // 3 = right mouse button

        unsigned int button = 0;

        switch( event->button() )
        {
        case Qt::LeftButton:
            button = 1;
            break;

        case Qt::MiddleButton:
            button = 2;
            break;

        case Qt::RightButton:
            button = 3;
            break;

        default:
            break;
        }

        if (impl_->getEventQueue()) 
            impl_->getEventQueue()->mouseButtonRelease( static_cast<float>( event->x() ),
            static_cast<float>( event->y() ),
            button );
    }
}

void Widget::wheelEvent( QWheelEvent* event )
{
    // Ignore wheel events as long as the selection is active.
    if( impl_->selectionActive_ )
        return;

    event->accept();
    int delta = event->delta();

    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
        : osgGA::GUIEventAdapter::SCROLL_DOWN;

    if (impl_->getEventQueue())
        impl_->getEventQueue()->mouseScroll( motion );
}

bool Widget::event( QEvent* event )
{
    bool handled = QWidget::event( event );

    // This ensures that the OSG widget is always going to be repainted after the
    // user performed some interaction. Doing this in the event handler ensures
    // that we don't forget about some event and prevents duplicate code.
    switch( event->type() )
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
        this->update();
        break;
    case QEvent::Close:
        impl_->engine_->GetViewer()->setDone(true);
        break;
    default:
        break;
    }

    return handled;
}



} // namespace visual
