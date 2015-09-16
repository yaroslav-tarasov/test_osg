#include "widget2.h"

#include <QBoxLayout>

#include "av/Visual.h"
#include <osgViewer/api/Win32/GraphicsWindowWin32>

namespace workaround
{
    void setCallBack(Window w, QObject* obj );
}

namespace {

#if defined _WIN32 || defined __CYGWIN__

    struct win_wnd
    {
        struct data
        {
            QObject* obj;
            WNDPROC  wpOrigEditProc;
        };

        typedef std::map< Window, data>  obj_map_t;
        typedef	std::set< std::pair<WPARAM,LPARAM> >  lock_pairs_t;
        friend void workaround::setCallBack(Window w, QObject* obj );

        static LRESULT CALLBACK vWndProc(HWND hwnd,   UINT uMsg,   WPARAM wParam,  LPARAM lParam)
        {
            uint16_t xPos = LOWORD(lParam);
            uint16_t yPos = HIWORD(lParam);
            static HCURSOR arrowCursor = LoadCursor(NULL, IDC_ARROW);
            static  lock_pairs_t lock_;

            auto w = getObjects()[hwnd].obj;
            switch (uMsg)
            {

            case WM_KILLFOCUS  :
                {
                    // Workaround для проблемы с потерей фокуса при при сворачивании окна
                    QEvent event(QEvent::WindowActivate);
                    QApplication::sendEvent(w, &event);
                }
                break;

            case WM_DESTROY:
                {
                    getObjects().erase(getObjects().find(hwnd));
                }
                break;
            }    

            return  CallWindowProc(getObjects()[hwnd].wpOrigEditProc, hwnd, uMsg, 
                wParam, lParam);     
        };

    private:
        static obj_map_t& getObjects()
        {
            static obj_map_t obj_map; 
            return obj_map;
        }
    };
#endif


}

namespace workaround
{
    void setCallBack(Window w, QObject* obj )
    {
#if defined _WIN32 || defined __CYGWIN__
        win_wnd::data d;

        d.wpOrigEditProc = (WNDPROC)SetWindowLong(static_cast<HWND>(w), GWL_WNDPROC, (LONG)win_wnd::vWndProc);
        d.obj = obj;
        win_wnd::getObjects().insert(std::make_pair<Window,win_wnd::data>(w,d));
#endif
    }
}

namespace visual
{

//
// Base widget
//

struct Widget2::WidgetPImpl
{
    WidgetPImpl( CoreWidget * widget_base, int x, int y, int width, int height )
        : selectionActive_(false)
        , selectionFinished_(false)
    {
        render_active_ = false;

        // OSG graphics context
        osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits();
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
        pTraits->samples                       = 8;
        pTraits->x = x;
        pTraits->y = y;
        pTraits->width = width;
        pTraits->height = height;
        
        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(pTraits.get());
        if (!gc)
        {
            osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
            return;
        }

        graphicsWindow_ = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());

        QWindow* native_wnd  = QWindow::fromWinId((WId)dynamic_cast<osgViewer::GraphicsHandleWin32*>(graphicsWindow_.get())->getHWND());
        
        native_wdgt_  = QWidget::createWindowContainer(native_wnd, widget_base);
        
        workaround::setCallBack(dynamic_cast<osgViewer::GraphicsHandleWin32*>(graphicsWindow_.get())->getHWND(), widget_base);

        engine_ = CreateVisual();

        osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(engine_->GetViewer());
        osgViewer::ViewerBase::Cameras cams_;
        v->getCameras(cams_,false);
        cams_[0]->setGraphicsContext( graphicsWindow_.get() );
        cams_[0]->setViewport(new osg::Viewport(0, 0, width, height ));


        widget_base->setLayout(new QBoxLayout(QBoxLayout::LeftToRight));
        widget_base->layout()->setContentsMargins(0, 0, 0, 0);
        widget_base->layout()->addWidget(native_wdgt_);

        widget_base->setFocusProxy(native_wdgt_);
        widget_base->setFocusPolicy( Qt::StrongFocus );
        widget_base->setMouseTracking( true );
 
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
        if(getEventQueue())
            getEventQueue()->windowResize( x, y, width, height );

        if(graphicsWindow_.valid())
            graphicsWindow_->resized( x, y, width, height );

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
            cameras[0]->setViewport(new osg::Viewport( 0, 0, width, height) );
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
    
    osg::ref_ptr<osgViewer::GraphicsWindow>			    graphicsWindow_;

    QPoint		    selectionStart_;
    QPoint		    selectionEnd_;

    bool			selectionActive_;
    bool			selectionFinished_;

    QWidget*        native_wdgt_;
};



// ctor
Widget2::Widget2()
{
    impl_ = new WidgetPImpl(this,
        this->x(),
        this->y(),
        this->width(),
        this->height());
}

// dtor
Widget2::~Widget2()
{
    delete impl_;
}

//
// visual_widget
//


void Widget2::createScene()
{

}

void Widget2::endSceneCreation()
{

}

void Widget2::redraw()
{
    QWidget::repaint();
}

//
// Main overloaded events
//

void Widget2::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event)

    // protect from cyclic draws
    if (!impl_->render_active_)
    {
        // render in
        impl_->render_active_ = true;
        impl_->engine_->Update();
        impl_->engine_->Render();

    }
    // render out
    impl_->render_active_ = false;
}

void Widget2::resizeEvent( QResizeEvent * event )
{
    impl_->resizeGL( 0, 0, event->size().width(), event->size().height() );
}

void Widget2::changeEvent( QEvent * event )
{
    switch (event->type())
    {
    case QEvent::ParentChange:
        LogWarn("Widget::QEvent::ParentChange");
        // reattach_to_window(WId(impl_->context_->createCompatibleWindow(Window(winId()))));
        break;
    default:
        break;
    }
}


void Widget2::keyPressEvent( QKeyEvent* event )
{
#if 0
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

#endif
}

void Widget2::keyReleaseEvent( QKeyEvent* event )
{
#if 0
    QString keyString   = event->text();
    const char* keyData = keyString.toLocal8Bit().data();

    if (impl_->getEventQueue()) impl_->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );

#endif
}

void Widget2::mouseMoveEvent( QMouseEvent* event )
{
#if 0
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
#endif
}

void Widget2::mousePressEvent( QMouseEvent* event )
{
#if 0
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
#endif
}

void Widget2::mouseReleaseEvent(QMouseEvent* event)
{
#if 0
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
#endif
}

void Widget2::wheelEvent( QWheelEvent* event )
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

bool Widget2::event( QEvent* event )
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
    case QEvent::WindowActivate:
        impl_->native_wdgt_->setFocus();
        break;
    case QEvent::Close:
        impl_->engine_->GetViewer()->setDone(true);
        break;
    default:
        break;
    }

    force_log fl;
    if(event->type()!=77 && event->type() != 12 && event->type() != 5 && event->type() != 173)
        LOG_ODS_MSG( "Widget::event( QEvent* event ):" << event->type() << "\n" );

    return handled;
}


} // namespace visual
