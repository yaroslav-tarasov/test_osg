#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgSim/MultiSwitch>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgGA/TrackballManipulator>
#include <osgViewer/CompositeViewer>
#include <osgGA/StateSetManipulator>

#include "findNodeVisitor.h"
#include "FollowNodeManip.h"

void createView (osgViewer::CompositeViewer *viewer,
                 osg::ref_ptr<osg::Group> scene,
                 osg::ref_ptr<osg::GraphicsContext> gc,
                 osgGA::TrackballManipulator* Tman,
                 int x, int y, int width, int height)
{
            double left,right,top,bottom,near,far, aspectratio;
            double frusht, fruswid, fudge;
            bool gotfrustum;
            osgViewer::View* view = new osgViewer::View;
            viewer -> addView(view);
            view->setCameraManipulator(Tman);

            view->setSceneData(scene.get());
            view->getCamera()->setViewport(new osg::Viewport(x,y, width,height));
            view->getCamera()-> getProjectionMatrixAsFrustum(left,right,
                                          bottom,top,
                                          near,far);
            if (gotfrustum) 
              {
              aspectratio = (double) width/ (double) height; 
              frusht = top - bottom;
              fruswid = right - left;
              fudge = frusht*aspectratio/fruswid;
              right = right*fudge;
              left = left*fudge;
              view->getCamera()-> setProjectionMatrixAsFrustum(left,right,
                                          bottom,top,
                                          near,far);
              }
            view->getCamera()->setGraphicsContext(gc.get());

            // add the state manipulator
            osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
            statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

            view->addEventHandler( statesetManipulator.get() );
}
