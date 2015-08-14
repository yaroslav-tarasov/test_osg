#include "stdafx.h"

#include "Grass.h"


namespace avTerrain
{

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



Grass::Grass ()
{

	_instances	= 1024.0f;		// grid cells _instances*_instances // 40 bydef
	_spacing		= 0.2f;		// spacing
	_heightAdjust	= 0.0f;
	_grassStretch	= 0.1f;
	_windFactor	= 0.0f;

	_width = 1024/**4*//*width*/;
	_height = 1024/**4*//*height*/;

	this->addChild(_create());
}

void Grass::setWindFactor(float wf)
{
	_windFactor = wf;
	unfWindFactor_->set(wf);
}

void Grass::setGrassMapFactor(float val)
{
	unfGrassMapFactor_->set(val);
}

osg::Node*  Grass::_create()
{
	/*osg::Geode*		geodeGrass*/_geodeGrass = new osg::Geode();	
	osg::ref_ptr<osg::Geometry> geomGrass = _createGeometry();
	
	_geodeGrass->setCullingActive( false );   

	float len = (_instances/2.0f)*_spacing;
	//osg::BoundingBox bb( -len, -len, 0.0f, len, len, 1.0f );
	//geomGrass->setInitialBound( bb );
	_geodeGrass->addDrawable( geomGrass.get() );

	_buildStateSet(_geodeGrass);

	osg::PositionAttitudeTransform* rPat = new osg::PositionAttitudeTransform;
	rPat->setPosition( osg::Vec3(0, 0, 0) );
	rPat->addChild( _geodeGrass );
	
	_createMap();
	
	return rPat;
}

void   Grass::_buildStateSet(osg::Node* node)
{
    std::string grass_vert = database::LoadShader("grass/grass.vert");     
    
    if (grass_vert.empty())
        return;

    std::string grass_frag = database::LoadShader("grass/grass.frag");     

    if (grass_frag.empty())
        return;


    osg::Shader* vertexShader = new osg::Shader();
    vertexShader->setType( osg::Shader::VERTEX );
    //vertexShader->loadShaderSourceFromFile( osgDB::findDataFile("grass/grass.vert") );
    vertexShader->setShaderSource(grass_vert);

    osg::Shader* fragShader = new osg::Shader();
    fragShader->setType( osg::Shader::FRAGMENT );
    //fragShader->loadShaderSourceFromFile( osgDB::findDataFile("grass/grass.frag") );
    fragShader->setShaderSource(grass_frag);

    osg::Program*  program = new osg::Program();
    program->addShader( vertexShader );
    program->addShader( fragShader );

	osg::StateSet* ss = node->getOrCreateStateSet();
	ss->setAttribute( program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

	unfInstances_ = new osg::Uniform("cc",(int)_instances);
	ss->addUniform( 	unfInstances_ );

	// height map
	//	ss->setTextureAttributeAndModes( 1, pRTT->getTexture(), osg::StateAttribute::ON );
	unfHeightMap_ = new osg::Uniform( "texHeightMap", 1 );
	ss->addUniform( unfHeightMap_ );

	// camera lock
	unfLock_ = new osg::Uniform( "bLock", false );
	ss->addUniform( unfLock_ );

	// _spacingacing
	unfSpacing_ = new osg::Uniform( "spacing", (float)_spacing );
	ss->addUniform( unfSpacing_ );

	// height adjust
	unfHeightAdjust_ = new osg::Uniform( "heightAdjust", _heightAdjust );
	ss->addUniform( unfHeightAdjust_ );

	// wind factor
	unfWindFactor_ = new osg::Uniform( "windFactor", _windFactor );
	ss->addUniform( unfWindFactor_ );

	// grass stretch
	unfGrassStretch_ = new osg::Uniform( "grassStretch", _grassStretch );
	ss->addUniform( unfGrassStretch_ );

	unfGrassMapFactor_  = new osg::Uniform( "grassMapFactor", 0.125f);
    ss->addUniform( unfGrassMapFactor_ );

	osg::Texture2D* pTex = new osg::Texture2D;
	osg::Image* pImage = osgDB::readImageFile( osgDB::findDataFile("grass/grass2.tga") );
	pTex->setImage( pImage );
	ss->setTextureAttributeAndModes( 0, pTex, osg::StateAttribute::ON );
	ss->setMode( GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	ss->setRenderBinDetails( 9/*RENDER_BIN_SCENE*/, "DepthSortedBin" );

	ss->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	ss->setAttribute( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.9f), osg::StateAttribute::ON );

	ss->setNestRenderBins(false);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
}

osg::Geometry* Grass::_createGeometry()
{
	osg::Geometry* geom = new osg::Geometry;
	geom->setUseDisplayList( false );
	geom->setUseVertexBufferObjects( true );
	createDAIGeometry( *geom, _instances*_instances );

	return geom;
}


//create noise map with values ranging from 0 to 255
bool Grass::_createMap()
{
	{
		osg::StateSet* stateset = _geodeGrass->getOrCreateStateSet();
		
		stateset->addUniform( new osg::Uniform("texMap", 7));

		_map = new osg::Texture2D;
		_map->setTextureSize(_width, _height);
		_map->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		_map->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
		//_map->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		//_map->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		stateset->setTextureAttributeAndModes(7, _map.get(), osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);

	}

	osg::Image* image = new osg::Image;
	unsigned char* data = new unsigned char[_width*_height];
	unsigned char* tmpData = new unsigned char[_width*_height];

	// int random=rand() % 5000;
	for(unsigned y=0; y < _height; y++)
		for(unsigned x=0; x < _width; x++)
		{
			// data[y*_width + x] = (unsigned char) (0.5 * 255.0 + Utility::getNoise(x, y, random) * 0.5 * 255.0);
			data[y*_width + x] = (unsigned char) (/*0.5 */ (y - _height *.5) * (y - _height *.5) +(x - _width *.5)*(x - _width *.5) < 100000? 255.0:0.0 /*+ Utility::getNoise(x, y, random) * 0.5 * 255.0*/);
		}
#if 0
	//if style isn't crayon style, smooth the noise map
	if(!_isCrayon)
	{
		{
			for(unsigned y=0; y < _height; y++)
				for(unsigned i=0; i < 4; i++)
					for(unsigned x=0; x < _width; x++)
						tmpData[y*_width + x] = (unsigned char)Utility::smoothNoise(_width, _height,x,y, data);

			for(unsigned y=0; y < _height; y++)
				for(unsigned x=0; x < _width; x++)
					data[y*_width + x] = (unsigned char)Utility::smoothNoise(_width, _height, x,y, tmpData);
		}
	}
#endif

	image->setImage(_width, _height, 1,
		1, GL_LUMINANCE, GL_UNSIGNED_BYTE,
		data,
		osg::Image::USE_NEW_DELETE);
	_map->setImage(image);
	return true;
}


}