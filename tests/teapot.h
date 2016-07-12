#pragma once

#include <GL/glut.h>

class TeapotDrawable : public osg::Drawable
{
public:
    TeapotDrawable( float size=1.0f ) : _size(size) {}
    TeapotDrawable( const TeapotDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
        : osg::Drawable(copy, copyop), _size(copy._size) {}

    META_Object( osg, TeapotDrawable );

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
	virtual osg::BoundingBox computeBoundingBox() const
#else
	virtual osg::BoundingBox computeBound() const
#endif
    {
        osg::Vec3 _min(-_size,-_size,-_size), _max(_size, _size, _size);
        return osg::BoundingBox(_min, _max);
    }

    virtual void drawImplementation( osg::RenderInfo& /*renderInfo*/ ) const
    {
        glFrontFace( GL_CW );
        glutSolidTeapot( _size );
        glFrontFace( GL_CCW );
    }

protected:
    float _size;
};