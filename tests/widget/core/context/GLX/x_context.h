#include "../base_context.h"

#include <QX11Info>

namespace core {

class XContext : public BaseContext
{
    Display * display;
    GLXFBConfig config;
    GLXContext gl3Context;

public:

    XContext()
    {
        Init();
    }

private:

    static GLXFBConfig chooseConfig( Display * display )
    {
        static int visualAttribs[] =
        {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_DOUBLEBUFFER    , True,
            GLX_RED_SIZE        , 8,
            GLX_GREEN_SIZE      , 8,
            GLX_BLUE_SIZE       , 8,
            GLX_ALPHA_SIZE      , 8,
            GLX_DEPTH_SIZE      , 24,
            GLX_STENCIL_SIZE    , 8,
            GLX_SAMPLE_BUFFERS  , True,
            GLX_SAMPLES         , 4,
            None
        };

        int fbcount = 0;
        GLXFBConfig *fbc = ::glXChooseFBConfig(display, DefaultScreen(display), visualAttribs, &fbcount);
        if(fbc == NULL)
        {
            LogError("No valid framebuffer config.");
        }
        GLXFBConfig config = fbc[0]; // Store the GLXFBConfig so that it can be used to create GLXContext later.
        XFree(fbc);
        return config;
    }

    void Init()
    {
        openglInit();

        display = basic_widget.x11Info().display();
        config = chooseConfig(display);

        widgetInit();

        gl3Context = glXCreateContextAttribsARB(display, config, 0, True, getContextAttributes());

        if(gl3Context == NULL)
            LogError("Unable to create gl context.");
        if(::glXIsDirect(display, gl3Context) != True)
            LogError("The gl context is not direct.");

        check_extensions_and_init_defaults();
    }


public:

    Window createCompatibleWindow( Window current )
    {
        Window window = 0;
        Window parent, root, *children;
        unsigned int count;

        XVisualInfo *vi = ::glXGetVisualFromFBConfig(display, config);

        if (current != 0)
        {
            if (XQueryTree(display, current, &root, &parent, &children, &count) == 0)
                LogError("Xlib internal error");
        }
        else
            parent = RootWindow(display, vi->screen);

        {
            XSetWindowAttributes swa;
            swa.colormap          = XCreateColormap(display, parent, vi->visual, AllocNone);
            swa.background_pixmap = None;
            swa.border_pixel      = 0;

            window = XCreateWindow(display, parent, 0, 0, 100, 100, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap, &swa);
            if(window == 0)
            {
                LogError("Unable to create window.");
            }
        }
        return window;
    }

    void doMakeCurrent( CoreWidget const * widget ) const
    {
        if (widget == NULL)
            widget = &basic_widget;

        if (!glXMakeCurrent(display, widget->winId(), gl3Context))
            LogError("Unable to make current");
    }

    void doDoneCurrent() const
    {
        glXMakeCurrent(display, 0, 0);
    }

    void doSwapBuffers( CoreWidget const * widget ) const
    {
        glXSwapBuffers(display, widget->winId());
    }

    bool isCurrent( CoreWidget const * widget ) const
    {
        if (gl3Context != glXGetCurrentContext())
            return false;

        if (widget != NULL && widget->winId() != glXGetCurrentDrawable())
                return false;

        return true;
    }

};

}
