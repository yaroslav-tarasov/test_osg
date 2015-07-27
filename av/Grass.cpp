#include "stdafx.h"

#include "Grass.h"

namespace avTerrain
{


float cc			= 1024.0f;		// grid cells cc*cc
float spacing		= 0.2f;			// spacing
float heightAdjust	= 0.0f;
float windFactor	= 1.0f;
float grassStretch	= 0.1f;

void createDAIGeometry( osg::Geometry& geom, int nInstances=1 )
{
	const float halfDimX( .5 );
	const float halfDimZ( .5 );

	osg::Vec3Array* v = new osg::Vec3Array;
	v->resize( 4 );
	geom.setVertexArray( v );

	// Geometry for a single quad.
	(*v)[ 0 ] = osg::Vec3( -halfDimX, 0., 0. );
	(*v)[ 1 ] = osg::Vec3( halfDimX, 0., 0. );
	(*v)[ 2 ] = osg::Vec3( halfDimX, 0., halfDimZ*2.0f );
	(*v)[ 3 ] = osg::Vec3( -halfDimX, 0., halfDimZ*2.0f );


	// create color array data (each corner of our triangle will have one color component)
	osg::Vec4Array* pColors = new osg::Vec4Array;
	pColors->push_back( osg::Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	pColors->push_back( osg::Vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
	pColors->push_back( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
	pColors->push_back( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
	geom.setColorArray( pColors );

	// make sure that our geometry is using one color per vertex
	geom.setColorBinding( osg::Geometry::BIND_PER_VERTEX );

	osg::Vec2Array* pTexCoords = new osg::Vec2Array( 4 );
	(*pTexCoords)[0].set( 0.0f, 0.0f );
	(*pTexCoords)[1].set( 1.0f, 0.0f );
	(*pTexCoords)[2].set( 1.0f, 1.0f );
	(*pTexCoords)[3].set( 0.0f, 1.0f );
	geom.setTexCoordArray( 0, pTexCoords );

	// Use the DrawArraysInstanced PrimitiveSet and tell it to draw 1024 instances.
	geom.addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4, nInstances ) );
}

osg::Geometry* createGrassGeom()
{
	osg::Geometry* geom = new osg::Geometry;
	geom->setUseDisplayList( false );
	geom->setUseVertexBufferObjects( true );
	createDAIGeometry( *geom, cc*cc );

	return geom;
}

Grass::Grass ()
{
	this->addChild(_create());
}

osg::Node*  Grass::_create()
{
	osg::Geode*		geodeGrass = new osg::Geode();	
	osg::ref_ptr<osg::Geometry> geomGrass = createGrassGeom();
	
	
	geodeGrass->setCullingActive( false );   
	//////////////////////////
	// grass billboard stuff
	//////////////////////////

	float len = (cc/2.0f)*spacing;
	osg::BoundingBox bb( -len, -len, 0.0f, len, len, 1.0f );
	geomGrass->setInitialBound( bb );
	geodeGrass->addDrawable( geomGrass.get() );

	//osg::StateSet* ss = _buildStateSet(geodeGrass);
	osg::Shader* vertexShader = new osg::Shader();
	vertexShader->setType( osg::Shader::VERTEX );
	vertexShader->loadShaderSourceFromFile( "grass.vert" );

	osg::Shader* fragShader = new osg::Shader();
	fragShader->setType( osg::Shader::FRAGMENT );
	fragShader->loadShaderSourceFromFile( "grass.frag" );

	osg::Program*  program = new osg::Program();
	program->addShader( vertexShader );
	program->addShader( fragShader );

	osg::StateSet* ss = _buildStateSet(geodeGrass);

	osg::Texture2D* pTex = new osg::Texture2D;
	osg::Image* pImage = osgDB::readImageFile( "grass2.tga" );
	pTex->setImage( pImage );
	ss->setTextureAttributeAndModes( 0, pTex, osg::StateAttribute::ON );
	ss->setMode( GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	ss->setRenderBinDetails( 9/*RENDER_BIN_SCENE*/, "DepthSortedBin" );

	ss->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	ss->setAttribute( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.9f), osg::StateAttribute::ON );
	
	ss->setNestRenderBins(false);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);


	osg::PositionAttitudeTransform* rPat = new osg::PositionAttitudeTransform;
	rPat->setPosition( osg::Vec3(0, 0, 0) );
	rPat->addChild( geodeGrass );

	return rPat;
}

osg::StateSet*   Grass::_buildStateSet(osg::Node* node)
{
	osg::Shader* vertexShader = new osg::Shader();
	vertexShader->setType( osg::Shader::VERTEX );
	vertexShader->loadShaderSourceFromFile( "grass.vert" );

	osg::Shader* fragShader = new osg::Shader();
	fragShader->setType( osg::Shader::FRAGMENT );
	fragShader->loadShaderSourceFromFile( "grass.frag" );

	osg::Program*  program = new osg::Program();
	program->addShader( vertexShader );
	program->addShader( fragShader );

	osg::StateSet* ss = node->getOrCreateStateSet();
	ss->setAttribute( program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

	unfCC_ = new osg::Uniform("cc",(int)cc);
	ss->addUniform( unfCC_ );

	// height map
	//	ss->setTextureAttributeAndModes( 1, pRTT->getTexture(), osg::StateAttribute::ON );
	unfHeightMap_ = new osg::Uniform( "texHeightMap", 1 );
	ss->addUniform( unfHeightMap_ );

	// camera lock
	unfLock_ = new osg::Uniform( "bLock", false );
	ss->addUniform( unfLock_ );

	// spacingacing
	unf_spacing_ = new osg::Uniform( "spacing", (float)spacing );
	ss->addUniform( unf_spacing_ );

	// height adjust
	unfHeightAdjust_ = new osg::Uniform( "heightAdjust", heightAdjust );
	ss->addUniform( unfHeightAdjust_ );

	// wind factor
	unfWindFactor_ = new osg::Uniform( "windFactor", windFactor );
	ss->addUniform( unfWindFactor_ );

	// grass stretch
	unfGrassStretch_ = new osg::Uniform( "grassStretch", grassStretch );
	ss->addUniform( unfGrassStretch_ );

	return ss;
}

}