#pragma once

#include "visual/visual_widget.h"

#include <QPoint>
#include <QOpenGLWidget>
#include "openglwindow.h"

namespace visual
{

    class TriangleWindow 
        : public OpenGLWindow
        , public visual_widget 
    {
    public:
        TriangleWindow();
        
//   visual_widget impl
    public: 
        void         createScene();
        void         endSceneCreation();
        void         redraw();

    private:
        void initialize() Q_DECL_OVERRIDE;
        void render()     Q_DECL_OVERRIDE;    

    private:
        GLuint loadShader(GLenum type, const char *source);

        GLuint m_posAttr;
        GLuint m_colAttr;
        GLuint m_matrixUniform;

        QOpenGLShaderProgram *m_program;
        int m_frame;
    };


}



