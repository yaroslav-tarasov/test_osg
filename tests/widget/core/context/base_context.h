#pragma once

#include <gl/glew.h>

#ifdef WIN32
#include <gl/wglew.h>
#else
#include <gl/glxew.h>
#endif // WIN32

//@ для винды так, для линукса иначе
#ifdef WIN32
typedef HWND Window;
#endif


#include <QGLWidget>

namespace core {

//
// Core context interface
//

struct context
    : virtual intrusive_ptr_observed
{
    virtual Window createCompatibleWindow( Window current = 0 ) = 0;

    virtual void makeCurrent( CoreWidget const * widget = nullptr ) const = 0;
    virtual void doneCurrent() const = 0;
    virtual void swapBuffers( CoreWidget const * widget ) const = 0;

    virtual bool isCurrent( CoreWidget const * widget = nullptr ) const = 0;
};
typedef boost::intrusive_ptr<context> context_ptr;

//
// Base context impl
//

class BaseContext : public context
{
protected:

    CoreWidget basic_widget;

    mutable CoreWidget const * currentWidget;
    mutable uint lock_count;

    BaseContext()
        : currentWidget(nullptr)
        , lock_count(0)
    {
        LogInfo("GLChart context ctor");
    }
    ~BaseContext() { LogInfo("GLChart context dtor"); }

    void openglInit()
    {
        QGLWidget gl_widget;
        gl_widget.makeCurrent();
        ::glewInit();
        gl_widget.doneCurrent();
    }

    void widgetInit()
    {
        basic_widget.reattach_to_window(WId(createCompatibleWindow()));
    }

    // context
protected:
    void makeCurrent( CoreWidget const * widget ) const override
    {
        if (lock_count++ == 0 || (widget != nullptr && widget != currentWidget))
        {
            currentWidget = widget;
            doMakeCurrent(widget);
        }
    }
    void doneCurrent() const override
    {
        if (--lock_count == 0)
        {
            doDoneCurrent();
            currentWidget = nullptr;
        }
    }
    void swapBuffers( CoreWidget const * widget ) const override
    {
        if (widget == nullptr)
            LogError("Can't swap on default widget");
        if (widget != currentWidget)
            LogError("Make widget current before calling swapbuffers");
        doSwapBuffers(widget);
    }

    // own
protected:
    virtual void doMakeCurrent( CoreWidget const * widget = nullptr ) const = 0;
    virtual void doDoneCurrent() const = 0;
    virtual void doSwapBuffers( CoreWidget const * widget ) const = 0;

#ifdef WIN32
    #define GL_(Name) WGL_ ## Name
#else
    #define GL_(Name) GLX_ ## Name
#endif

    const int * getContextAttributes()
    {
        static const int ctxAttribList[] =
        {
            GL_(CONTEXT_MAJOR_VERSION_ARB), 3,
            GL_(CONTEXT_MINOR_VERSION_ARB), 3,
            GL_(CONTEXT_PROFILE_MASK_ARB), GL_(CONTEXT_CORE_PROFILE_BIT_ARB),
            //GL_(CONTEXT_FLAGS_ARB), GL_(CONTEXT_DEBUG_BIT_ARB),
            0,
        };
        return ctxAttribList;
    }


#ifdef WIN32
    #define GetProcAddress(str) wglGetProcAddress(str)
#else
    #define GetProcAddress(str) glXGetProcAddress((GLubyte *)str)
#endif

    void check_extensions_and_init_defaults()
    {
        doMakeCurrent();

        glewExperimental = GL_TRUE;
        ::glewInit();

        const GLubyte *s1 =  glGetString(GL_VENDOR);

        LogInfo("GL_VENDOR = " << glGetString(GL_VENDOR) << " GL_RENDERER = " << glGetString(GL_RENDERER));
        LogInfo("GL_SHADING_LANGUAGE_VERSION = " << glGetString(GL_SHADING_LANGUAGE_VERSION));


        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glEnable(GL_SCISSOR_TEST);

        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glClearDepthf(1.0f);
        glClearStencil(0);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glDisable(GL_DITHER);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(-1);

        glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLARBPROC)GetProcAddress("glDebugMessageControlARB");
        glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKARBPROC)GetProcAddress("glDebugMessageCallbackARB");
        //setDebugOutput();

        doDoneCurrent();
    }

    // some problems with glew linking under ubuntu
    PFNGLDEBUGMESSAGECONTROLARBPROC glDebugMessageControl;
    PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallback;

    void setDebugOutput( bool enable = true )
    {
        if (enable)
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            glDebugMessageCallback(&debugOutput, 0);
        }
        else
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    }

private:

#ifndef WIN32
    #define __stdcall
#endif

    static void __stdcall debugOutput( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam )
    {
        Q_UNUSED(length);
        Q_UNUSED(userParam);

        switch (id)
        {
        case 131220:
        case 131185:
        case 131186:
            // skip bufinfo and other shit
            return;
        }

        static const char *sourceDescrition[] = {
            "OpenGL", "Windows", "Shader Compiler", "Third Party", "Application", "Other"
        };
        static const char *typeDescrition[] = {
            "error", "deprecated behavior", "undefined behavior", "portability", "performance", "message"
        };
        static const char * severityDescrition[] = {
            "high", "medium", "low"
        };

        if (severity >= GL_DEBUG_SEVERITY_MEDIUM_ARB)
        {
            LogWarn("<" << sourceDescrition[source - GL_DEBUG_SOURCE_API_ARB] << ">["
                << typeDescrition[type - GL_DEBUG_TYPE_ERROR_ARB] << "]("
                << severityDescrition[severity - GL_DEBUG_SEVERITY_HIGH_ARB] << "){" << id << "}: "
                << message);
        } else if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
        {
            LogError("<" << sourceDescrition[source - GL_DEBUG_SOURCE_API_ARB] << ">["
                << typeDescrition[type - GL_DEBUG_TYPE_ERROR_ARB] << "]("
                << severityDescrition[severity - GL_DEBUG_SEVERITY_HIGH_ARB] << "){" << id << "}: "
                << message);
        }
    }

};

}
