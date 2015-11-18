//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avWeather/Cloud.h"

#define ATTRIB_FACTORS_LOCATION 6

//
// Module namespaces
//

using namespace avWeather;


//
// Local cloud constructor
//

// constructor
Cloud::Cloud(size_t nID)
    : m_fRadX(100.f)
    , m_fRadY(100.f)
    , m_fRadZ(100.f)
    , m_fHeight(100.f)
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
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    pCurStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

    // enable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
    pCurStateSet->setAttribute(pDepth);

    // disable cull-face just for the case
    pCurStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // setup shader
    osg::Program * pCurProgram = new osg::Program;
    pCurProgram->setName("LocalCloud");
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("LocalCloud.vs", NULL, osg::Shader::VERTEX  ));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("LocalCloud.gs", NULL, osg::Shader::GEOMETRY));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("LocalCloud.fs", NULL, osg::Shader::FRAGMENT));
    pCurProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pCurProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pCurProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

    pCurProgram->addBindAttribLocation( "Factors", ATTRIB_FACTORS_LOCATION );

    // bind shader
    pCurStateSet->setAttribute(pCurProgram);
    


    // uniforms

    // radii and heights
    m_uniformRadiiHeight = new osg::Uniform("RadiiHeight", osg::Vec4f());
    pCurStateSet->addUniform(m_uniformRadiiHeight.get());
    // inversed radii and particle size
    m_uniformInvRadiiSize = new osg::Uniform("InvRadiiSize", osg::Vec4f());
    pCurStateSet->addUniform(m_uniformInvRadiiSize.get());

    // color at bottom level
    m_uniformColorBottom = new osg::Uniform("ColorBottom", osg::Vec4f());
    pCurStateSet->addUniform(m_uniformColorBottom.get());
    // color at top level
    m_uniformColorTop = new osg::Uniform("ColorTop", osg::Vec4f());
    pCurStateSet->addUniform(m_uniformColorTop.get());

    // textures

    // setup texture for point quads
    pCurStateSet->setTextureAttribute(0, avCore::GetDatabase()->LoadTexture("CloudsAtlasGrey.dds", osg::Texture::CLAMP_TO_EDGE));

    //
    // Construct geometry
    //

    // number of particles
    static const size_t
        g_nLayersNum = 7,
        g_nLayerParticlsCount = 160;

    // create vertex array
    m_paEllipsePointPos = new osg::Vec3Array();
    // all other factors (type, size, rotation, intensity)
    osg::Vec4ubArray * paFactors = new osg::Vec4ubArray();


    // random number with bank identifier as initial value
    avCore::RandomNumber rndGen(nID);

    // height deviation
    const float fStepH = 1.0f / float(g_nLayersNum);
    // decide about axis bend
    const float
        fAxeBendAngle = rndGen.random_range(0.f, 2.0f * osg::PI),
        fAxeBendValue = 0.75f * fStepH * rndGen.random_unit();
    // axis bend delta vector
    const osg::Vec3f vAxeBend(fAxeBendValue * cosf(fAxeBendAngle), fAxeBendValue * sinf(fAxeBendAngle), fStepH);


    // no particles collected so far
    size_t nParticlesCollected = 0;
    // layer center position
    osg::Vec3f vLayerCenter;

    // go through all layers
    for (size_t i = 0; i < g_nLayersNum; ++i)
    {
        // squared-parabolic shape (not spherical)
        const float
            fCurH = i / float(g_nLayersNum),
            fCurRadScale = osg::square(1.0f - fCurH * fCurH);

        // when going up - less particles needed
        size_t nParticlesCurLayer = ceil(fCurRadScale * g_nLayerParticlsCount);

        // resize arrays by new layer
        m_paEllipsePointPos->resize(nParticlesCollected + nParticlesCurLayer);
        paFactors->resize(nParticlesCollected + nParticlesCurLayer);

        // fill layer points
        for (size_t j = 0; j < nParticlesCurLayer; ++j)
        {
            // save it
            osg::Vec3f & curVertex = m_paEllipsePointPos->at(nParticlesCollected + j);
            osg::Vec4ub & curFactors = paFactors->at(nParticlesCollected + j);
            // random position (when going up - tend to create particles on sides, awaring center)
            const float
                fR = fCurRadScale * rndGen.random_range(fCurH, 1.f),
                fAngle = rndGen.random_range(0.f, 2.0f * osg::PI);
            curVertex = vLayerCenter + osg::Vec3f(fR * cosf(fAngle), fR * sinf(fAngle), (i) ? fStepH * rndGen.random_unit() : 0.f);
            // random factors
            curFactors[0] = 10 + rndGen.random_32bit() % 6; // last 6 textures from atlas so far
            curFactors[1] = rndGen.random_8bit();
            curFactors[2] = rndGen.random_8bit();
            curFactors[3] = rndGen.random_8bit();
        }

        // some particles added
        nParticlesCollected += nParticlesCurLayer;
        // shift axis vector
        vLayerCenter += vAxeBend;
    }

    // set vertex array
    m_paEllipsePointPos->setDataVariance(osg::Object::STATIC);
    setVertexArray(m_paEllipsePointPos.get());

    // attach draw call command
    m_dipDrawElem = new osg::DrawElementsUShort(osg::PrimitiveSet::POINTS, nParticlesCollected);
    addPrimitiveSet(m_dipDrawElem.get());

    // set factors array
    static const size_t
        idxSpeedFactorsBinding = ATTRIB_FACTORS_LOCATION;
    paFactors->setDataVariance(osg::Object::STATIC);
    setVertexAttribBinding(idxSpeedFactorsBinding, osg::Geometry::BIND_PER_VERTEX);
    setVertexAttribNormalize(idxSpeedFactorsBinding, GL_TRUE);
    setVertexAttribArray(idxSpeedFactorsBinding, paFactors);


    // defaults
    SetEllipsoid(2000.f, 500.f, 300.f, 500.f);
    SetPrecipitationDensity(PrecipitationRain, 1.0f);

    // exit
    return;
}

//
// Base local cloud control
//

// set ellipsoid
void Cloud::SetEllipsoid(float fRadX, float fRadY, float fRadZ, float fHeight)
{
    // set radii and height
    if (!cg::eq(m_fRadX, fRadX) || !cg::eq(m_fRadY, fRadY) || !cg::eq(m_fRadZ, fRadZ) || !cg::eq(m_fHeight, fHeight))
    {
        // save new values
        m_fRadX = fRadX;
        m_fRadY = fRadY;
        m_fRadZ = fRadZ;
        m_fHeight = fHeight;

        // particle mean size
        const float fPartSize = 0.45f * osg::maximum(fRadX, fRadY);
        // increase height by size just to make clouds not intersect local fog so much
        m_fHeightCorrected = m_fHeight + 0.75f * fPartSize;
        

        // reset uniforms
        m_uniformRadiiHeight->set(osg::Vec4f(m_fRadX, m_fRadY, m_fRadZ, m_fHeightCorrected));
        m_uniformInvRadiiSize->set(osg::Vec4f(1.0f / m_fRadX, 1.0f / m_fRadY, 1.0f / m_fRadZ, fPartSize));

        // recalc bound
        m_aabbEllipsoid = osg::BoundingBox(
            osg::Vec3f(-m_fRadX - fPartSize, -m_fRadY - fPartSize, m_fHeightCorrected - fPartSize),
            osg::Vec3f(+m_fRadX + fPartSize, +m_fRadY + fPartSize, m_fHeightCorrected + m_fRadZ + fPartSize));
    }
}

// set precipitation type and density
void Cloud::SetPrecipitationDensity(PrecipitationType pType, float fDensity)
{
    // save new values
    m_ptType = pType;
    m_fDensity = fDensity;
}


//
// Special culling
//

// special culling method
void Cloud::cull(osgUtil::CullVisitor * pCV)
{
    // array with particles data
    struct tagSortData
    {
        float dist;
        GLushort idx;
        bool operator < (const tagSortData & sec) const { return dist < sec.dist; }
    };
    static std::vector<tagSortData> g_sortData;

    // particles number
    const size_t nParticlesNumber = m_paEllipsePointPos->size();
    g_sortData.resize(nParticlesNumber);

    // convert current eye point to current ellipse system
    const osg::Vec3f vEyePos = pCV->getEyeLocal();
    const osg::Vec3f vLocalEyePos = vEyePos - osg::Vec3f(0.f, 0.f, m_fHeightCorrected);

    // fill sort data
    for (size_t i = 0; i < nParticlesNumber; ++i)
    {
        static osg::Vec3f vDeltaPnt;
        vDeltaPnt = m_paEllipsePointPos->at(i);
        vDeltaPnt.x() *= m_fRadX;
        vDeltaPnt.y() *= m_fRadY;
        vDeltaPnt.z() *= m_fRadZ;
        vDeltaPnt -= vLocalEyePos;
        g_sortData[i].dist = vDeltaPnt.length2();
        g_sortData[i].idx = i;
    }
    // make sort
    std::sort(g_sortData.begin(), g_sortData.end());

    // save current order in draw elements
    for (size_t i = 0; i < nParticlesNumber; ++i)
        m_dipDrawElem->at(i) = g_sortData[nParticlesNumber - i - 1].idx;
    m_dipDrawElem->dirty();


    // calculate cloud transparency
    const float
        fCloudTranspLow  = cg::lerp01( 0.05f, 1.0f, m_fDensity),
        fCloudTranspHigh = cg::lerp01( 0.0f, 1.0f, m_fDensity * m_fDensity * (2.0f - m_fDensity));

    // calculate bottom-top cloud colors
    const osg::Vec3f cPrepColor = GetPrecipitationColor(m_ptType);
    m_uniformColorBottom->set(osg::Vec4f(cPrepColor * 0.940f, fCloudTranspLow));
    m_uniformColorTop->set(osg::Vec4f(cPrepColor * 1.185f, fCloudTranspHigh));


    // push drawable state set
    pCV->pushStateSet(getStateSet());
    // add drawable with precipitations
    pCV->addDrawableAndDepth(this, pCV->getModelViewMatrix(), (vEyePos - osg::Vec3f(0.f, 0.f, 1.5f * m_fHeight)).length());
    // pop drawable state set
    pCV->popStateSet();
}
