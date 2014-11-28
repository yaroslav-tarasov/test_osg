#include "stdafx.h"
#include <GL/glut.h>


class TeapotDrawable : public osg::Drawable
{
public:
    TeapotDrawable( float size=1.0f ) : _size(size) {}
    TeapotDrawable( const TeapotDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
    : osg::Drawable(copy, copyop), _size(copy._size) {}
    
    META_Object( osg, TeapotDrawable );
    
    virtual osg::BoundingBox computeBound() const
    {
        osg::Vec3 _min(-_size,-_size,-_size), _max(_size, _size, _size);
        return osg::BoundingBox(_min, _max);
    }
    
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const
    {
        glFrontFace( GL_CW );
        glutSolidTeapot( _size );
        glFrontFace( GL_CCW );
    }
    
protected:
    float _size;
};

int main_teapot( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( new TeapotDrawable(1.0f) );
    
    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( root.get() );
    return viewer.run();
}
