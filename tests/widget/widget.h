#pragma once

#include "visual/visual_widget.h"

#include "core/widget/core_widget.h"

#include <QResizeEvent>
//
// Widget base
//
namespace visual
{

class Widget 
    : public core::CoreWidget   
    , public visual_widget     
{
    Q_OBJECT

public:

    Widget();
    ~Widget();


public: // visual_widget interface

    //engine_factory * engine() const override;
    //scene_view * view() const override;

    void redraw() override;
    void createScene() override;
    void endSceneCreation() override;

protected:  // base Qt stuff to change

    virtual void paintEvent( QPaintEvent * event );
    virtual void resizeEvent( QResizeEvent * event );
    virtual void changeEvent( QEvent * event );


    virtual void keyPressEvent    ( QKeyEvent* event ) override;
    virtual void keyReleaseEvent  ( QKeyEvent* event ) override;

    virtual void mouseMoveEvent   ( QMouseEvent* event ) override;
    virtual void mousePressEvent  ( QMouseEvent* event ) override;
    virtual void mouseReleaseEvent( QMouseEvent* event ) override;
    virtual void wheelEvent       ( QWheelEvent* event ) override;
    
    virtual bool event            ( QEvent* event ) override;

    virtual QSize sizeHint () const { return QSize(512, 512); }

private:

    struct WidgetPImpl;
    WidgetPImpl * impl_;
};
} // ns glchart
