#ifndef OSGWidget_h__
#define OSGWidget_h__

#include <QPoint>
#include <QOpenGLWidget>

#ifdef OSGWIDGET_EXPORTS
# define OSGWIDGET_API __declspec(dllexport)
#else
# define OSGWIDGET_API __declspec(dllimport)
#endif

class OSGWIDGET_API OSGWidget : public QOpenGLWidget
{
  Q_OBJECT

public:
  OSGWidget( QWidget* parent = 0,
             Qt::WindowFlags f = 0 );

  virtual ~OSGWidget();

  void         createScene();
  void         endSceneCreation();
  void         redraw();

protected:

  virtual void paintEvent	    ( QPaintEvent* paintEvent );
  virtual void paintGL		    ();
  virtual void resizeGL         ( int width, int height );

  virtual void keyPressEvent    ( QKeyEvent* event );
  virtual void keyReleaseEvent  ( QKeyEvent* event );

  virtual void mouseMoveEvent   ( QMouseEvent* event );
  virtual void mousePressEvent  ( QMouseEvent* event );
  virtual void mouseReleaseEvent( QMouseEvent* event );
  virtual void wheelEvent       ( QWheelEvent* event );

  virtual bool event            ( QEvent* event );

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

#endif
