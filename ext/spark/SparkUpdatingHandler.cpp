#include "stdafx.h"
#include "SparkUpdatingHandler.h"
#include <osgViewer/View>

bool SparkUpdatingHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
    if ( !view ) return false;
    
    osg::Camera* camera = view->getCamera();
    if ( !camera ) return false;
    
    double time = view->getFrameStamp()->getSimulationTime();
    if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
    {
        osg::Vec3d eye, center, up;
        camera->getViewMatrixAsLookAt( eye, center, up );
        
        for ( std::vector<SparkObject>::iterator itr=_sparks.begin(); itr!=_sparks.end(); ++itr )
        {
            SparkDrawable* spark = itr->_spark.get();
            if ( !spark ) continue;
            
            osg::Transform* trackee = itr->_trackee.get();
            if ( trackee )
            {
                if ( itr->_dirtyMatrix )
                {
                    itr->_transformMatrix = computeTransformMatrix( spark, trackee );
                    itr->_dirtyMatrix = false;
                }
      
#if 1
                osg::NodePathList& lst = trackee->getParentalNodePaths();
                if(lst.size()==0)
                    return false;

                osg::NodePath& trackeePath = lst[0];
                //trackeePath.pop_back();
#endif
                osg::Matrix matrix = osg::computeLocalToWorld(trackeePath); //trackee->computeLocalToWorldMatrix(matrix, NULL);
                spark->setGlobalTransformMatrix( matrix /** itr->_transformMatrix*/ );
            }
            spark->update( time, eye );
        }
    }
    return false;
}

osg::Matrix SparkUpdatingHandler::computeTransformMatrix( SparkDrawable* spark, osg::Transform* trackee )
{
    osg::Node* sparkGeode = (spark->getNumParents()>0 ? spark->getParent(0) : NULL);
    if ( !sparkGeode )
        return osg::Matrix::identity();
    else if ( !sparkGeode->getNumParents() || !trackee->getNumParents() )
        return osg::Matrix::identity();
    else if ( sparkGeode->getParent(0)==trackee->getParent(0) )
        return osg::Matrix::identity();
    
    // Collect the parent paths, ignoring the last one (the spark/trackee itself)
    osg::NodePath& sparkPath = sparkGeode->getParentalNodePaths().size()>0?sparkGeode->getParentalNodePaths()[0]:osg::NodePath(); if(sparkPath.size()>0) sparkPath.pop_back();
    
    osg::NodePathList& lst = trackee->getParentalNodePaths();
    
    if(lst.size()==0)
        return osg::Matrix::identity();
    
    osg::NodePath& trackeePath = lst[0];
    trackeePath.pop_back();

    return computeLocalToWorld(trackeePath) * computeWorldToLocal(sparkPath);
}
