#pragma  once

#include <LinearMath/btIDebugDraw.h>
#include "common/debug_render.h"

namespace avCollision
{
/** \class GLDebugDrawer GLDebugDrawer.h <osgbCollision/GLDebugDrawer.h>
\brief Debug utility to render Bullet collision shapes.

The class visualizes Bullet collision shapes. Use it as a debugging aid
to ensure that Bullet collision shapes and OSG geometry are synchronized.
*/
class /*OSGBCOLLISION_EXPORT*/ GLDebugDrawer : public btIDebugDraw
{
public:
    GLDebugDrawer(debug_render_ptr dr);
    virtual ~GLDebugDrawer(){};

    // osg::Node* getSceneGraph();

    void setEnabled( bool enable );
    bool getEnabled() const;

    virtual void	drawLine( const btVector3& from,const btVector3& to,const btVector3& color );
    virtual void	drawSphere( const btVector3& p, btScalar radius, const btVector3& color );

    virtual void	drawTriangle(const btVector3& a,const btVector3& b,const btVector3& c,const btVector3& color,btScalar alpha);
    virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
    virtual void	reportErrorWarning(const char* warningString);
    virtual void	draw3dText(const btVector3& location,const char* textString);

    virtual void	setDebugMode(int debugMode);
    virtual int		getDebugMode() const;

    void EndDraw();
    void BeginDraw();

    void setTextSize( const float size );
    float getTextSize() const;

private:

    debug_render_ptr        dr_;
};


// avCollision
}


