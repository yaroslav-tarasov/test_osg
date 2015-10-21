#include <stdafx.h>
#include "av/precompiled.h"



#include "av/avCore/avCore.h"

#include "av/avScene/Scene.h"
#ifdef SCREEN_TEXTURE
#include "av/avScene/ScreenTextureManager.h"
#endif

#include "av/avWeather/FogBank.h"

//
// Module namespaces
//

using namespace avWeather;


//
// Local fog bank constructor
//

// constructor
LocalFogBank::LocalFogBank()
    : m_fRadX(2000.f)
    , m_fRadY(500.f)
    , m_fMaxH(500.f)
    , m_fDensity(1.0f)
    , m_fDensityLow(0.5f)
    , m_fPortion(0.5f)
    , m_ptType(PrecipitationRain)
{
    // base set up
    setUseDisplayList(false);
    setUseVertexBufferObjects(true);
    setDataVariance(osg::Object::STATIC);
    setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback());

    //
    // create state set
    //

    osg::StateSet * pCurStateSet = getOrCreateStateSet();

    // setup render-bin details
    pCurStateSet->setNestRenderBins(false);
    pCurStateSet->setRenderBinDetails(RENDER_BIN_LOCAL_WEATHER, "DepthSortedBin");

    // setup blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    pCurStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

    // disable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false);
    pCurStateSet->setAttribute(pDepth);

    // enable cull-face, but we draw back edges, that's the trick for convex bodies!
    pCurStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

    // enable depth clamping to avoid cutting
    pCurStateSet->setMode(GL_DEPTH_CLAMP_NV, osg::StateAttribute::ON);

    // setup shader
    osg::Program * pCurProgram = new osg::Program;
    pCurProgram->setName("FogBank");
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("FogBank.vs", NULL, osg::Shader::VERTEX  ));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("FogBank.fs", NULL, osg::Shader::FRAGMENT));
    // bind shader
    pCurStateSet->setAttribute(pCurProgram);

    // uniforms

    // radii and heights
    m_uniformRadiiHeight = new osg::Uniform("RadiiHeight", osg::Vec3f());
    pCurStateSet->addUniform(m_uniformRadiiHeight.get());
    // inverted radii and density functions
    m_uniformRcpRadiiHeight = new osg::Uniform("RcpRadiiHeight", osg::Vec3f());
    pCurStateSet->addUniform(m_uniformRcpRadiiHeight.get());
    // density control
    m_uniformDensity = new osg::Uniform("DensityControl", osg::Vec4f());
    pCurStateSet->addUniform(m_uniformDensity.get());
    // density control
    m_uniformColor = new osg::Uniform("Color", osg::Vec3f());
    pCurStateSet->addUniform(m_uniformColor.get());

#ifdef SCREEN_TEXTURE
    // depth texture
    avScene::ScreenTextureManager * pScrTexManager = avScene::Scene::GetInstance()->getScreenTextureManager();
    m_texDepth = pScrTexManager->request(RENDER_BIN_AFTER_MODELS, avScene::ScreenTexture::DEPTH_TEXTURE);
    static const int g_nDepthTexUnit = 0;
    pCurStateSet->setTextureAttribute(g_nDepthTexUnit, m_texDepth->getTexture());
    pCurStateSet->addUniform(new osg::Uniform("DepthTexture", g_nDepthTexUnit));
    pCurStateSet->addUniform(m_texDepth->getSettings());
#endif

    // reflection stateset
    m_ssReflectionSubstitute = new osg::StateSet();
    m_ssReflectionSubstitute->setNestRenderBins(false);
    m_ssReflectionSubstitute->setRenderBinDetails(RENDER_BIN_LOCAL_WEATHER, "DepthSortedBin");
    m_ssReflectionSubstitute->addUniform(new osg::Uniform("DepthTextureSettings", osg::Vec2f(0.0, -0.0001)));

    //
    // Construct geometry
    //

    static const size_t g_nRadialSubd = 96;

    // create vertex array
    osg::Vec2Array * paCylinderPoints = new osg::Vec2Array(g_nRadialSubd * 2);
    for (size_t i = 0; i < g_nRadialSubd; i++)
        paCylinderPoints->at(i).set((2.0f * osg::PI * i) / g_nRadialSubd, 0.f);
    for (size_t i = 0; i < g_nRadialSubd; i++)
        paCylinderPoints->at(g_nRadialSubd + i).set((2.0f * osg::PI * i) / g_nRadialSubd, 1.f);
    paCylinderPoints->setDataVariance(osg::Object::STATIC);
    setVertexArray(paCylinderPoints);

    static const size_t g_nCapTris = g_nRadialSubd - 2;
    static const size_t g_nSideTris = g_nRadialSubd * 2;

    // create index array
    osg::DrawElementsUShort * paDrawElem = new osg::DrawElementsUShort(GL_TRIANGLES, (g_nSideTris + 2 * g_nCapTris) * 3);
    size_t nStartTriIdx = 0;
    for (size_t i = 0; i < g_nRadialSubd; i++)
    {
        paDrawElem->at(nStartTriIdx + 0) = i;
        paDrawElem->at(nStartTriIdx + 1) = g_nRadialSubd + (i + 1) % g_nRadialSubd;
        paDrawElem->at(nStartTriIdx + 2) = (i + 1) % g_nRadialSubd;
        nStartTriIdx += 3;

        paDrawElem->at(nStartTriIdx + 0) = i;
        paDrawElem->at(nStartTriIdx + 1) = g_nRadialSubd + i;
        paDrawElem->at(nStartTriIdx + 2) = g_nRadialSubd + (i + 1) % g_nRadialSubd;
        nStartTriIdx += 3;
    }
    for (size_t i = 0; i < g_nCapTris; i++)
    {
        paDrawElem->at(nStartTriIdx + 0) = 0;
        paDrawElem->at(nStartTriIdx + 1) = i + 1;
        paDrawElem->at(nStartTriIdx + 2) = i + 2;
        nStartTriIdx += 3;
    }
    for (size_t i = 0; i < g_nCapTris; i++)
    {
        paDrawElem->at(nStartTriIdx + 0) = g_nRadialSubd + 0;
        paDrawElem->at(nStartTriIdx + 1) = g_nRadialSubd + i + 2;
        paDrawElem->at(nStartTriIdx + 2) = g_nRadialSubd + i + 1;
        nStartTriIdx += 3;
    }
    avAssert(nStartTriIdx == (g_nSideTris + 2 * g_nCapTris) * 3);
    addPrimitiveSet(paDrawElem);


    // initialize
    _recalcBound();
    _resetUniforms();

    // exit
    return;
}

//
// Base local fog bank control
//

// set bank radii and height (min height is assuming as tide level, which is zero for Environment node, under which we are placed)
void LocalFogBank::SetEllipse( float fRadX, float fRadY, float fMaxH )
{
    // set radii and height
    if (!cg::eq(m_fRadX, fRadX) || !cg::eq(m_fRadY, fRadY) || !cg::eq(m_fMaxH, fMaxH))
    {
        // save new values
        m_fRadX = fRadX;
        m_fRadY = fRadY;
        m_fMaxH = fMaxH;
        // reset
        _resetUniforms();
        _recalcBound();
    }
}

// set density and portion
void LocalFogBank::SetPrecipitationDensityPortion( PrecipitationType ptType, float fInnerInt, float fPortion, float fLowerBoundDensityScale )
{
    // save precipitation type
    m_ptType = ptType;

    // set radii
    if (!cg::eq(m_fDensity, fInnerInt) || !cg::eq(m_fPortion, fPortion))
    {
        // save new values
        m_fDensity = fInnerInt;
        m_fPortion = osg::minimum(fPortion, 0.999999f);
        m_fDensityLow = fLowerBoundDensityScale;
        // uniforms
        _resetUniforms();
    }
}

//
// Special culling
//

// special culling method
void LocalFogBank::cull( osgUtil::CullVisitor * pCV )
{
    // exit if reflections
    const bool bMagicReflPath = (pCV->getCullMask() == 0x00010000UL);

    // calculate color
    m_uniformColor->set(GetPrecipitationColor(m_ptType));

#ifdef SCREEN_TEXTURE
    // validate depth matrix
    if (!bMagicReflPath)
        m_texDepth->validate();
#endif

    // save view to local
    m_mViewToLocal = osg::Matrix::inverse(*pCV->getModelViewMatrix());

    // push drawable state set
    pCV->pushStateSet(getStateSet());
    if (bMagicReflPath)
        pCV->pushStateSet(m_ssReflectionSubstitute.get());

    // add drawable
    pCV->addDrawableAndDepth(this, pCV->getModelViewMatrix(), (pCV->getEyeLocal() - m_aabbCylinder.center()).length());

    // pop drawable state set
    if (bMagicReflPath)
        pCV->popStateSet();
    pCV->popStateSet();
}


//
// Drawing impl
//

void LocalFogBank::drawImplementation( osg::RenderInfo & renderInfo ) const
{
    // set up tricky blend
    osg::GL2Extensions * gl2e = osg::GL2Extensions::Get(renderInfo.getState()->getContextID(), true);
    if (gl2e)
        gl2e->glBlendEquationSeparate(/*0x8006*/GL_FUNC_ADD, /*0x8008*/GL_MAX);    

    // call what we have to
    osg::Geometry::drawImplementation(renderInfo);

    // restore defaults
    if (gl2e)
        gl2e->glBlendEquationSeparate(/*0x8006*/GL_FUNC_ADD, /*0x8006*/GL_FUNC_ADD);
}

//
// Internal methods
//

// AABB calculation routine
void LocalFogBank::_recalcBound()
{
    m_aabbCylinder = osg::BoundingBox(osg::Vec3f(-m_fRadX, -m_fRadY, 0.0f), osg::Vec3f(+m_fRadX, +m_fRadY, m_fMaxH));
}

// uniforms update routine
void LocalFogBank::_resetUniforms()
{
    m_uniformRadiiHeight->set(osg::Vec3f(m_fRadX, m_fRadY, m_fMaxH));
    m_uniformRcpRadiiHeight->set(osg::Vec3f(1.f / m_fRadX, 1.f / m_fRadY, 1.f / m_fMaxH));
    m_uniformDensity->set(osg::Vec4f(m_fPortion / (1.0f - m_fPortion), 0.f, m_fDensity, m_fDensityLow));
}
