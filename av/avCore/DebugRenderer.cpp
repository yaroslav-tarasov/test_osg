#include "stdafx.h"

#include "DebugRenderer.h"
#include <osgbCollision/Utils.h>

#include <iostream>

#include <stdio.h> //printf debugging



namespace avCollision
{


////////////////////////////////////////////////////////////////////////////////
DebugRenderer::DebugRenderer()
  : _enabled( true ),
    _active( false ),
    _textSize( 1.f ),
    _textStrings( 0 ),
    _frame( 0 ),
    _contacts( 0 )
{
    setDebugMode( ~0u );

    _group = new osg::Group();
    _group->setName( "Bullet Debug" );

    _geode = new osg::Geode();
    _geode->setName( "Bullet pts, lns, tris, and text" );
    _geode->setDataVariance( osg::Object::DYNAMIC );
    {
        osg::StateSet* ss = _geode->getOrCreateStateSet();
        ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    }
    _group->addChild( _geode.get() );


    _ptGeom = new osg::Geometry;
    _ptGeom->setDataVariance( osg::Object::DYNAMIC );
    _ptGeom->setUseDisplayList( false );
    _ptGeom->setUseVertexBufferObjects( false );
    {
        osg::StateSet* ss = _geode->getOrCreateStateSet();
        ss->setMode( GL_POINT_SMOOTH, osg::StateAttribute::ON );
        osg::Point* point = new osg::Point( 20. );
        ss->setAttributeAndModes( point, osg::StateAttribute::ON );
    }
    _geode->addDrawable( _ptGeom.get() );

    _ptVerts = new osg::Vec3Array();
    _ptGeom->setVertexArray( _ptVerts );
    _ptColors = new osg::Vec4Array();
    _ptGeom->setColorArray( _ptColors );
    _ptGeom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );


    _lnGeom = new osg::Geometry;
    _lnGeom->setDataVariance( osg::Object::DYNAMIC );
    _lnGeom->setUseDisplayList( false );
    _lnGeom->setUseVertexBufferObjects( false );
    _geode->addDrawable( _lnGeom.get() );

    _lnVerts = new osg::Vec3Array();
    _lnGeom->setVertexArray( _lnVerts );
    _lnColors = new osg::Vec4Array();
    _lnGeom->setColorArray( _lnColors );
    _lnGeom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );


    _triGeom = new osg::Geometry;
    _triGeom->setDataVariance( osg::Object::DYNAMIC );
    _triGeom->setUseDisplayList( false );
    _triGeom->setUseVertexBufferObjects( false );
    _geode->addDrawable( _triGeom.get() );

    _triVerts = new osg::Vec3Array();
    _triGeom->setVertexArray( _triVerts );
    _triColors = new osg::Vec4Array();
    _triGeom->setColorArray( _triColors );
    _triGeom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );


    // Initialize _textVec to display 10 text strings; resize later if necessary
    _textVec.resize( 10 );
    int idx;
    for( idx=0; idx<10; idx++ )
        _textVec[ idx ] = initText();

}

DebugRenderer::~DebugRenderer()
{
    while( _group->getNumParents() > 0 )
        _group->getParent( 0 )->removeChild( _group.get() );
}


osg::Node*
DebugRenderer::getSceneGraph()
{
    return( _group.get() );
}

void DebugRenderer::setEnabled( bool enable )
{
    if( !enable)
    {
        // Clear all geometry.
        _enabled = true;
        BeginDraw();
        // Disable any other rendering (like the chart)
        _group->setNodeMask( 0x0 );
    }
    else
    {
        _group->setNodeMask( /*0xffffffff*/0x00010000 );
    }
    _enabled = enable;
}
bool DebugRenderer::getEnabled() const
{
    return( _enabled );
}

////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::drawLine(const osg::Vec3& from,const osg::Vec3& to,const osg::Vec3& color)
{
    if( !getEnabled() )
        return;

    if( !_active)
    {
        osg::notify( osg::WARN ) << "DebugRenderer: BeginDraw was not called." << std::endl;
        return;
    }

    // If the physics sim contains a plane, the AABB is rendered with
    // astronomical values. When this occurs in combination with OSG's
    // auto-compute near/far feature, the resulting far plane is very
    // distant, and consequently the near plane is pulled back to maintain
    // the default near/far ratio. As a result, the entire scene is clipped.
    // In this case, don't draw this line.
    osg::Vec3 osgFrom = from; //osgbCollision::asOsgVec3( from );
    osg::Vec3 osgTo = to;     //osgbCollision::asOsgVec3( to );
    const double bigValue( 10000. );
    if( ( osg::absolute< double >( osgFrom[ 0 ] ) > bigValue ) ||
        ( osg::absolute< double >( osgFrom[ 1 ] ) > bigValue ) ||
        ( osg::absolute< double >( osgFrom[ 2 ] ) > bigValue ) ||
        ( osg::absolute< double >( osgTo[ 0 ] ) > bigValue ) ||
        ( osg::absolute< double >( osgTo[ 1 ] ) > bigValue ) ||
        ( osg::absolute< double >( osgTo[ 2 ] ) > bigValue ) )
        return;
    _lnVerts->push_back( osgFrom );
    _lnVerts->push_back( osgTo );  

    osg::Vec4 c = osg::Vec4(color,1); //osgbCollision::asOsgVec4( color, 1. );
    _lnColors->push_back( c );
    _lnColors->push_back( c );
}

void DebugRenderer::drawSphere( const osg::Vec3& p, float radius, const osg::Vec3& color )
{
    if( !getEnabled() )
        return;

    if( !_active )
    {
        osg::notify( osg::WARN ) << "DebugRenderer: BeginDraw was not called." << std::endl;
        return;
    }

    osg::notify( osg::ALWAYS ) << "DebugRenderer::drawASphere NYI" << std::endl;
}


////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::drawTriangle(const osg::Vec3& a,const osg::Vec3& b,const osg::Vec3& c,const osg::Vec3& color,float alpha)
{
    if( !getEnabled() )
        return;

    if( !_active)
    {
        osg::notify( osg::WARN ) << "DebugRenderer: BeginDraw was not called." << std::endl;
        return;
    }

    _triVerts->push_back( a /*osgbCollision::asOsgVec3( a )*/ );
    _triVerts->push_back( b /*osgbCollision::asOsgVec3( b )*/ );
    _triVerts->push_back( c /*osgbCollision::asOsgVec3( c )*/ );

    osg::Vec4 c4 = osg::Vec4( color, alpha); // osgbCollision::asOsgVec4( color, alpha );
    _triColors->push_back( c4 );
    _triColors->push_back( c4 );
    _triColors->push_back( c4 );
}
////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::draw3dText(const osg::Vec3& location,const char* textString)
{
    if( !getEnabled() )
        return;

    FIXME(Bullet Text)
    //if( (_debugMode & btIDebugDraw::DBG_DrawText) == 0 )
    //    return;

    if( !_active )
    {
        osg::notify( osg::WARN ) << "DebugRenderer: BeginDraw was not called." << std::endl;
        return;
    }

    if( _textStrings == _textVec.size() )
    {
        int oldSize( _textVec.size() );
        int newSize( oldSize * 2 );
        _textVec.resize( newSize );
        int idx;
        for( idx=oldSize; idx<newSize; idx++ )
            _textVec[ idx ] = initText();
    }
    osgText::Text* text = _textVec[ _textStrings ].get();
    _textStrings++;

    text->setPosition( location /*osgbCollision::asOsgVec3( location )*/ );
    text->setText( std::string( textString ) );

    _geode->addDrawable( text );
}
////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::reportErrorWarning(const char* warningString)
{
    if( !getEnabled() )
        return;

    logger::need_to_log(true);

    LOG_ODS_MSG( warningString << std::endl );

    logger::need_to_log(false);

    osg::notify( osg::WARN ) << warningString << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::drawContactPoint( const osg::Vec3& pointOnB,
    const osg::Vec3& normalOnB, float distance, 
    int lifeTime, const osg::Vec3& color)
{
    if( !getEnabled() )
        return;

    if( !_active )
    {
        osg::notify( osg::WARN ) << "DebugRenderer: BeginDraw was not called." << std::endl;
        return;
    }

    _contacts++;

    _ptVerts->push_back( pointOnB /*osgbCollision::asOsgVec3( pointOnB )*/ );
    _ptColors->push_back( osg::Vec4( color, 1.) /*osgbCollision::asOsgVec4( color, 1. )*/ );

    /*btVector3*/osg::Vec3 to=pointOnB+normalOnB*distance;
    const /*btVector3*/osg::Vec3 &from = pointOnB;

    drawLine( from, to, color );

    char buf[12];
    sprintf(buf," %d",lifeTime);

    draw3dText( from, buf );
}
////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::BeginDraw()
{
    if( !getEnabled() )
        return;

    if( _ptVerts->size() > 0 )
    {
        _ptGeom->removePrimitiveSet( 0 );
        _ptVerts->clear();
        _ptColors->clear();
    }

    if( _lnVerts->size() > 0 )
    {
        _lnGeom->removePrimitiveSet( 0 );
        _lnVerts->clear();
        _lnColors->clear();
    }

    if( _triVerts->size() > 0 )
    {
        _triGeom->removePrimitiveSet( 0 );
        _triVerts->clear();
        _triColors->clear();
    }

    if( _geode->getNumDrawables() > 3 )
        _geode->removeDrawables( 3, _textStrings );
    _textStrings = 0;

    _contacts = 0;

    _active = true;
}
////////////////////////////////////////////////////////////////////////////////
void DebugRenderer::EndDraw()
{
    if( !getEnabled() )
        return;

    _active = false;

    if( _ptVerts->size() )
        _ptGeom->addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, _ptVerts->size() ) );
    if( _lnVerts->size() )
        _lnGeom->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, _lnVerts->size() ) );
    if( _triVerts->size() )
        _triGeom->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, _triVerts->size() ) );

    _frame++;
}
////////////////////////////////////////////////////////////////////////////////

void DebugRenderer::setDebugMode(int debugMode)
{
    _debugMode = debugMode;
}
int DebugRenderer::getDebugMode() const
{
    return( _debugMode );
}

void DebugRenderer::setTextSize( const float size )
{
    _textSize = size;
}
float DebugRenderer::getTextSize() const
{
    return( _textSize );
}

osgText::Text*
DebugRenderer::initText()
{
    osgText::Text* text = new osgText::Text;
    text->setDataVariance( osg::Object::DYNAMIC );
    text->setFont( "fonts/arial.ttf" );
    text->setColor( osg::Vec4( 1., 1., 1., 1. ) );
    text->setCharacterSize( _textSize );
    text->setAxisAlignment( osgText::Text::SCREEN );

     return text;
}



// osgbCollision
}
