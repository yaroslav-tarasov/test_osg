#ifndef OSGWidget_h__
#define OSGWidget_h__

#include "visual/visual_widget.h"

#include <QPoint>
#include <QOpenGLWidget>

namespace visual
{

class VISUAL_API OSGWidget 
    : public QOpenGLWidget
    , public visual_widget 
{
  Q_OBJECT

public:
  OSGWidget( QWidget* parent = 0,
             Qt::WindowFlags f = 0 );

  virtual ~OSGWidget();

//   visual_widget impl
public: 
  void         createScene();
  void         endSceneCreation();
  void         redraw();

protected:

  virtual void paintEvent	    ( QPaintEvent* paintEvent );
  virtual void paintGL		    ();
  virtual void resizeGL         ( int width, int height );

  virtual void keyPressEvent    ( QKeyEvent* event ) override;
  virtual void keyReleaseEvent  ( QKeyEvent* event ) override;

  virtual void mouseMoveEvent   ( QMouseEvent* event ) override;
  virtual void mousePressEvent  ( QMouseEvent* event ) override;
  virtual void mouseReleaseEvent( QMouseEvent* event ) override;
  virtual void wheelEvent       ( QWheelEvent* event ) override;

  virtual bool event            ( QEvent* event ) override;

private:

  virtual void onHome			();
  virtual void onResize			( int width, int height );
  void		   processSelection	();

private :
  struct osg_private;

  osg_private* d_;

  QPoint		selectionStart_;
  QPoint		selectionEnd_;

  bool			selectionActive_;
  bool			selectionFinished_;


};


}



#endif
