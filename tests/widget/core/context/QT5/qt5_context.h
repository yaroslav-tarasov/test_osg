#include <QOpenGLContext>
#include <QWindow>
#include <QSurface>

namespace core {

    class Qt5Context : public BaseContext
    {
        mutable QOpenGLContext context;

    public:

        Qt5Context()
        {
            Init();
        }

    private:

        void setPixelFormat( HDC hdc )
        {
            static const int attribList[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, 32,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 8,
                WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                WGL_SAMPLES_ARB, 8,
                0,        //End
            };

            int pixelFormat;
            uint numFormats;
            if (!wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats))
                LogError("Can't choose pixel format.");
            if (!SetPixelFormat(hdc, pixelFormat, NULL))
                LogError("Can't set pixel format.");
        }

        void Init()
        {
            openglInit();   //@base
            widgetInit();   //@base
                        
            //context = wglCreateContextAttribsARB(basic_widget.getDC(), 0, getContextAttributes());
            
            QSurfaceFormat fmt;
            const int *attrs = getContextAttributes();
            fmt.setAlphaBufferSize(8);
            fmt.setBlueBufferSize(8);
            fmt.setGreenBufferSize(8);
            fmt.setRedBufferSize(8);
            fmt.setDepthBufferSize(24);
            fmt.setStencilBufferSize(8);
            fmt.setSamples(8);
            fmt.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
            fmt.setMajorVersion(attrs[1]);  // дебилизм, св€занный с совмесимостью с предыдущим кодом
            fmt.setMinorVersion(attrs[3]);
            //fmt.setProfile()
            context.setFormat(fmt);
            
            context.create();
            
            check_extensions_and_init_defaults();   //@base
        }

    public:

        Window createCompatibleWindow( Window current )
        {
            // это нужно - дл€ винды остаетс€ как есть, это всего лишь создание окна
            WNDCLASS wndCls;

            /* create a dummy window */
            ZeroMemory(&wndCls, sizeof(wndCls));
            wndCls.lpfnWndProc = DefWindowProc;
            wndCls.hInstance = GetModuleHandle(NULL);
            wndCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
            wndCls.lpszClassName = L"VISUAL";
            RegisterClass( &wndCls );

            HWND parent = 0;
            if (current != 0)
                parent = GetParent(current);

            Window window = CreateWindow(L"VISUAL", NULL, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | (parent == 0 ? WS_OVERLAPPEDWINDOW : WS_CHILD), 30, 30, 700, 700, parent, NULL, GetModuleHandle(NULL), NULL);
            
            //@ внимание, здесь неверно, нам нужно не ƒ÷ а что-то другое, св€занное с surface?
            
            setPixelFormat(GetWindowDC(window));
            return window;
        }

        void doMakeCurrent( CoreWidget const * widget ) const
        {
            if (widget == NULL)
                widget = &basic_widget;

            QWindow *w = widget->windowHandle();
            if(!context.makeCurrent(w))
                LogError("Can't make context current");

//@             saved_dc_ = widget->getDC();
//@             if (!wglMakeCurrent(saved_dc_, context))
//@                 LogError("Can't make context current");
        }

        void doDoneCurrent() const
        {
            context.doneCurrent();

        //@    if (!wglMakeCurrent(saved_dc_, 0))
        //@        LogError("Can't release current thread context");
        //@    saved_dc_ = nullptr;
        }
        void doSwapBuffers( CoreWidget const * widget ) const
        {
        //@    Assert(saved_dc_ == widget->getDC());
        //@    SwapBuffers(saved_dc_);
            QWindow *w = widget->windowHandle();
            context.swapBuffers(w);
        }

        bool isCurrent( CoreWidget const * widget ) const
        {
            if (&context != QOpenGLContext::currentContext() )
                return false;

            if (widget != NULL && widget->windowHandle() != context.surface())
                return false;

            return true;
        }
    };
}
