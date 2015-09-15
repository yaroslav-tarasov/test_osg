#pragma once

#include "visual/visual_widget.h"

#include <QPoint>
#include <QOpenGLWidget>
#include "openglwindow.h"

struct IVisual;

namespace visual
{

    class OSGWidget2 
        : public OpenGLWindow
        , public visual_widget 
    {
    public:
        OSGWidget2();
        
//   visual_widget impl
    public: 
        void         createScene();
        void         endSceneCreation();
        void         redraw();

    private:
        void initialize() Q_DECL_OVERRIDE;
        void render()     Q_DECL_OVERRIDE;    

    private:
        IVisual*            engine_;
    };


}



