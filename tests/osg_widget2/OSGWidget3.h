#pragma once

#include "visual/visual_widget.h"

#include <QPoint>
#include <QOpenGLWidget>
#include "openglwindow.h"
#include "utils/high_res_timer.h"

struct IVisual;

namespace visual
{

    class OSGWidget3 
        : public QWidget
        , public visual_widget 
    {
    public:
        OSGWidget3();
        
//   visual_widget impl
    public: 
        void         createScene();
        void         endSceneCreation();
        void         redraw();

        virtual QSize sizeHint () const Q_DECL_OVERRIDE { return QSize(512, 512); };

    protected:
        bool event(QEvent *event) Q_DECL_OVERRIDE;
    private:
        void initialize();

    private:
        IVisual*            engine_;
        QWidget*            child_;
        high_res_timer                _hr_timer;
    };


}



