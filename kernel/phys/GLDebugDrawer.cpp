#include "stdafx.h"

#include "GLDebugDrawer.h"
#include <osgbCollision/Utils.h>

namespace avCollision
{


////////////////////////////////////////////////////////////////////////////////
GLDebugDrawer::GLDebugDrawer(debug_render* dr)
  : dr_( dr )
{

}


//osg::Node*
//GLDebugDrawer::getSceneGraph()
//{
//    return( dr_->getSceneGraph() );
//}

void GLDebugDrawer::setEnabled( bool enable )
{
    dr_->setEnabled(enable);
}
bool GLDebugDrawer::getEnabled() const
{
    return( dr_->getEnabled() );
}

////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
    dr_->drawLine(osgbCollision::asOsgVec3( from ),osgbCollision::asOsgVec3( to ),osgbCollision::asOsgVec3( color ));
}

void GLDebugDrawer::drawSphere( const btVector3& p, btScalar radius, const btVector3& color )
{
    dr_->drawSphere(osgbCollision::asOsgVec3( p ), radius,osgbCollision::asOsgVec3( color ));
}


////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::drawTriangle(const btVector3& a,const btVector3& b,const btVector3& c,const btVector3& color,btScalar alpha)
{
    dr_->drawTriangle(osgbCollision::asOsgVec3( a ),osgbCollision::asOsgVec3( b ),osgbCollision::asOsgVec3( c ),osgbCollision::asOsgVec3( color ), alpha);
}
////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::draw3dText(const btVector3& location,const char* textString)
{
    dr_->draw3dText(osgbCollision::asOsgVec3( location ), textString);
}
////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::reportErrorWarning(const char* warningString)
{
    dr_->reportErrorWarning(warningString);
}
////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::drawContactPoint( const btVector3& pointOnB,
    const btVector3& normalOnB, btScalar distance, 
    int lifeTime, const btVector3& color)
{
    dr_->drawContactPoint(osgbCollision::asOsgVec3( pointOnB ),osgbCollision::asOsgVec3( normalOnB ), distance, lifeTime, osgbCollision::asOsgVec3( color ));

}
////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::BeginDraw()
{
    dr_->BeginDraw();
}
////////////////////////////////////////////////////////////////////////////////
void GLDebugDrawer::EndDraw()
{
    dr_->EndDraw();
}
////////////////////////////////////////////////////////////////////////////////

void GLDebugDrawer::setDebugMode(int debugMode)
{
    dr_->setDebugMode(debugMode);
}
int GLDebugDrawer::getDebugMode() const
{
    return( dr_->getDebugMode() );
}

void GLDebugDrawer::setTextSize( const float size )
{
    dr_->setTextSize( size );
}
float GLDebugDrawer::getTextSize() const
{
    return( dr_->getTextSize() );
}


// avCollision
}
