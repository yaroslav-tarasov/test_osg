//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2009 Kongsberg Maritime AS, Marine IT Division Simulation
// All rights reserved.
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <osg/BlendFunc>
#include "Clouds.h"
#include "svScene/Scene.h"

using namespace avSky;

//////////////////////////////////////////////////////////////////////////
class CloudsLayerUniform : public osg::Uniform::Callback
{
public:
    CloudsLayerUniform( utils::uint nID ) : m_nLayerID( nID )
    {
    }

private:
    utils::uint    m_nLayerID;
    virtual void    operator () (osg::Uniform* uniform, osg::NodeVisitor* nv)
    {
        float fCloudsDensity = utils::GetEnvironment()->GetWeatherParameters().CloudDensity;

        switch( m_nLayerID )
        {
        case 0:
            uniform->set( osg::Vec4( 0.0, clamp( fCloudsDensity * 0.333f, 0.0f, 1.0f ), 1, 1 ) );
            break;
        case 1:
            uniform->set( osg::Vec4( 2500.0, clamp( (fCloudsDensity-0.333f) * 3.333f, 0.0f, 1.0f ), -2, 2 ) );
            break;
        case 2:
            uniform->set( osg::Vec4( 5000.0, clamp( (fCloudsDensity-0.666f) * 3.333f, 0.0f, 1.0f ), 3, -3 ) );
            break;
        default:
            svError( "Unknwon clouds layer" );
            break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////
Clouds::Clouds()
{
	Initialize();
}

//////////////////////////////////////////////////////////////////////////
bool Clouds::Initialize()
{
	// Load model
	osg::ref_ptr<osg::Node>  cCloudsNodePtr0 = utils::GetDatabase()->LoadModel( "CloudsLayer.obj" );
    osg::ref_ptr<osg::Node>  cCloudsNodePtr1 = utils::GetDatabase()->LoadModel( "CloudsLayer.obj" );
    osg::ref_ptr<osg::Node>  cCloudsNodePtr2 = utils::GetDatabase()->LoadModel( "CloudsLayer.obj" );
	addChild( cCloudsNodePtr0.get() );
    addChild( cCloudsNodePtr1.get() );
    addChild( cCloudsNodePtr2.get() );

	// Create shader
	osg::ref_ptr<osg::Program> cCloudsProgram = new osg::Program; 
	cCloudsProgram->setName( "CloudsShader" );
	cCloudsProgram->addShader( /*utils::GetDatabase()->LoadShader( "Clouds.vs" )*/ 
        osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("Clouds.vs")) );
	cCloudsProgram->addShader( /*utils::GetDatabase()->LoadShader( "Clouds.fs" )*/
        osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("Clouds.fs")) );
	osg::StateSet* cCloudsStateSet = getOrCreateStateSet();
	cCloudsStateSet->setAttribute( cCloudsProgram.get() );

	// Set render bin
	cCloudsStateSet->setRenderBinDetails( RENDER_BIN_CLOUDS, "RenderBin" );
    cCloudsStateSet->setNestRenderBins(false);

	// Create uniforms
	_cCloudsParamsPtr = cCloudsStateSet->getOrCreateUniform( "vParams", osg::Uniform::FLOAT_VEC4 );
	_cCloudsParamsPtr->setDataVariance( osg::Object::DYNAMIC );

	_cCloudsMaskPtr = cCloudsStateSet->getOrCreateUniform( "vColorMask", osg::Uniform::FLOAT_VEC4 );
	_cCloudsMaskPtr->setDataVariance( osg::Object::DYNAMIC );

	_cCloudsColorPtr = cCloudsStateSet->getOrCreateUniform( "vColor", osg::Uniform::FLOAT_VEC3 );
	_cCloudsColorPtr->setDataVariance( osg::Object::DYNAMIC );

    osg::ref_ptr<osg::Uniform> cCloudsLayer2 = cCloudsNodePtr0->getOrCreateStateSet()->getOrCreateUniform( "vCloudsLayer", osg::Uniform::FLOAT_VEC4 );
    cCloudsLayer2->setUpdateCallback( new CloudsLayerUniform( 2 ) );
    cCloudsLayer2->setDataVariance(osg::Object::DYNAMIC);

    osg::ref_ptr<osg::Uniform> cCloudsLayer1 = cCloudsNodePtr1->getOrCreateStateSet()->getOrCreateUniform( "vCloudsLayer", osg::Uniform::FLOAT_VEC4 );
    cCloudsLayer1->setUpdateCallback( new CloudsLayerUniform( 1 ) );
    cCloudsLayer1->setDataVariance(osg::Object::DYNAMIC);

    osg::ref_ptr<osg::Uniform> cCloudsLayer0 = cCloudsNodePtr2->getOrCreateStateSet()->getOrCreateUniform( "vCloudsLayer", osg::Uniform::FLOAT_VEC4 );
    cCloudsLayer0->setUpdateCallback( new CloudsLayerUniform( 0 ) );
    cCloudsLayer0->setDataVariance(osg::Object::DYNAMIC);

    // Enable alpha blending
    osg::BlendFunc* pBlend = new osg::BlendFunc;
    pBlend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    cCloudsStateSet->setAttributeAndModes( pBlend, osg::StateAttribute::ON );

    // Disable back-face culling
    cCloudsStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	// Set textures
	utils::SetTextureUniform( "CloudsScrollingMask.tga", "CloudsMask", 0, cCloudsStateSet );
	utils::SetTextureUniform( "CloudsScrolling.tga", "CloudsTexture", 1, cCloudsStateSet );

    // Disable culling
    utils::DisableCulling( this );

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Clouds::FrameCall()
{
    int iCloudType = utils::GetEnvironment()->GetWeatherParameters().CloudType;
    float fCloudsDensity = utils::GetEnvironment()->GetWeatherParameters().CloudDensity;
	float fTime = (float)fmod( utils::GetTimer()->GetSimulationTime(), 1000.0 );

	osg::Vec4 vParams;
	osg::Vec4 vMask;
	osg::Vec3 vColor;

	switch( iCloudType )
	{
	case 0:
		vMask = osg::Vec4( 1.0, 0.0, 0.0, 0.0 );
        vColor = lerp01( fCloudsDensity, osg::Vec3( 0.48, 0.49, 0.5 ), osg::Vec3( 0.38, 0.39, 0.4 ) );
		vParams[ 0 ] = 2.0f;
		vParams[ 1 ] = fTime * 0.002f;
		vParams[ 2 ] = 0.5f;
		vParams[ 3 ] = fTime * 0.004f;
		break;
	case 1:
		vMask = osg::Vec4( 0.0, 1.0, 0.0, 0.0 );
		vColor = lerp01( fCloudsDensity, osg::Vec3( 0.95, 0.98, 1.0 ), osg::Vec3( 0.85, 0.88, 0.9 ) );
		vParams[ 0 ] = 5.0f;
		vParams[ 1 ] = fTime * 0.001f;
		vParams[ 2 ] = 2.5f;
		vParams[ 3 ] = fTime * 0.002f;
		break;
	case 2:
		vMask = osg::Vec4( 0.0, 0.0, 1.0, 0.0 );
		vColor = lerp01( fCloudsDensity, osg::Vec3( 0.76, 0.784, 0.8 ), osg::Vec3( 0.66, 0.684, 0.7 ) );
		vParams[ 0 ] = 10.0f;
		vParams[ 1 ] = fTime * 0.004f;
		vParams[ 2 ] = 5.0f;
		vParams[ 3 ] = fTime * 0.008f;
		break;
	case 3:
		vMask = osg::Vec4( 0.0, 0.0, 0.0, 1.0 );
		vColor = lerp01( fCloudsDensity, osg::Vec3( 0.95, 0.98, 1.0 ), osg::Vec3( 0.85, 0.88, 0.9 ) );
		vParams[ 0 ] = 8.0f;
		vParams[ 1 ] = fTime * 0.02f;
		vParams[ 2 ] = 10.0f;
		vParams[ 3 ] = fTime * 0.04f;
		break;
	default:
		svError( "Unknown clouds type" );
		break;
	}

	_cCloudsParamsPtr->set( vParams );
	_cCloudsMaskPtr->set( vMask );
    const float fSunInt = svScene::GetScene()->getSky() ? svScene::GetScene()->getSky()->GetSunIntensity() : 1.0f;
	_cCloudsColorPtr->set( vColor * fSunInt );

	return true;
}
