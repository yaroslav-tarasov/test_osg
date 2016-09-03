//
// Module includes
//

#include "av/avCore/avCore.h"
#include "av/avCore/Global.h"

#include "av/avScene/Scene.h"
#include "av/avWeather/Weather.h"

#include "av/avLights/CulturalLights/PointLightsManager.h"

//
// Module namespaces
//

using namespace avScene;
using namespace avLights;

//
// Class, implementing drawable for succeeding lights info collecting
//

class PointLightsManager::CollectedCultLights : public osg::Geometry
{
public:

    // constructor
    __forceinline CollectedCultLights(PointLightsManager * pFather)
        : m_pFatherRef(pFather)
        , m_nAllocatedLightsNum(0)
        , m_nCollectedLightsNum(0)
        , m_fScreenClarity(0.f)
        , m_bVertHack(false)
    {
        // base set up
        setUseDisplayList(false);
        setUseVertexBufferObjects(true);
        setDataVariance(osg::Object::DYNAMIC);
        setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback());

        // make appropriate vertex buffers
        m_bufVertexData = new osg::Vec3Array();
        m_bufSizeDistData = new osg::Vec2Array();
        m_bufColorData = new osg::Vec4ubArray();

        // make them be somehow allocated at startup
        static const size_t g_nStartAllocatedNum = 128;
        _resizeBuffers(g_nStartAllocatedNum);

        // attach draw call command
        m_dipDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::POINTS);

        // save it into geometry
        setVertexArray(m_bufVertexData.get());
        setTexCoordArray(0, m_bufSizeDistData.get());
        setColorArray(m_bufColorData.get());
        setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        addPrimitiveSet(m_dipDrawArrays.get());

        // get that geometry state set
        osg::StateSet * pGeomStateSet = getOrCreateStateSet();
        pGeomStateSet->setNestRenderBins(false);
        pGeomStateSet->setRenderBinDetails(RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin");

        // create setting uniform
        m_uniformSettings = new osg::Uniform("LightScreenSettings", osg::Vec4f());
        pGeomStateSet->addUniform(m_uniformSettings.get());

        // exit
        return;
    }

    // set screen clarity
    __forceinline void setScreenClarity(float clarity)
    {
        m_fScreenClarity = clarity;
    }

    // set vertical reflections hack
    __forceinline void setReflectionsFlag(bool bRefl)
    {
        m_bVertHack = bRefl;
    }

    // set uniform
    __forceinline void saveUniform()
    {
        // vertical elongation
        const float
            fVertScale = m_bVertHack ? 5.0f : 1.0f;
        // total size
        const float
            fTotalSize = (m_bVertHack ? 5.0f : 2.50f);
        // distance for global reflections fading
        const float
            fDistFalloff = m_bVertHack ? cg::lerp01(m_pFatherRef->m_fFogDensityCur, 3250.f, 200.f) : std::numeric_limits<float>::max();
        // vis distance inverse multiplier when fog increases
        const float
            fDistFogFactor = m_pFatherRef->m_fVisDistCur;

        // save uniform
        m_uniformSettings->set(osg::Vec4f(m_fScreenClarity * fTotalSize, fVertScale, fDistFalloff, fDistFogFactor));
    }

    // add light
    __forceinline void addLight(const osg::Vec3f & vViewPos, const osg::Vec4ub & cColor, float fSize, float fVisDist)
    {
        // first of all - increment number of allocated lights, if needed
        if (m_nCollectedLightsNum >= m_nAllocatedLightsNum)
        {
            // too much memory - exit
            static const size_t g_nMaxVisibleLightsNumber = 8192;
            if (m_nAllocatedLightsNum >= g_nMaxVisibleLightsNumber)
                return;

            // twice the memory needed
            _resizeBuffers(m_nAllocatedLightsNum << 1);
        }

        // o'key, memory allocated, so add the point
        m_bufVertexData->at(m_nCollectedLightsNum) = vViewPos;
        m_bufSizeDistData->at(m_nCollectedLightsNum) = osg::Vec2f(fSize, fVisDist);
        m_bufColorData->at(m_nCollectedLightsNum) = cColor;

        // dirty arrays
        m_bufVertexData->dirty();
        m_bufSizeDistData->dirty();
        m_bufColorData->dirty();

        // increment collected lights number
        ++m_nCollectedLightsNum;
        // and update number of visible point
        m_dipDrawArrays->setCount(m_nCollectedLightsNum);
    }

    // flush lights
    __forceinline void flushLights()
    {
        // empty collected data
        m_nCollectedLightsNum = 0;
        m_dipDrawArrays->setCount(0);
    }

private:

    // resize buffers
    void _resizeBuffers(size_t new_size)
    {
        // save number
        m_nAllocatedLightsNum = new_size;
        // reallocate arrays
        m_bufVertexData->resize(m_nAllocatedLightsNum);
        m_bufSizeDistData->resize(m_nAllocatedLightsNum);
        m_bufColorData->resize(m_nAllocatedLightsNum);
        // exit
        return;
    }

    // father reference
    PointLightsManager * m_pFatherRef;

    // screen clarity (how many meters in pixel on a distance of 1 m in front of viewer)
    float m_fScreenClarity;
    // vertical reflection tune flag
    bool m_bVertHack;

    // numbers info
    size_t m_nAllocatedLightsNum;
    size_t m_nCollectedLightsNum;

    // vertex buffers for keeping lights data
    osg::ref_ptr<osg::Vec3Array>   m_bufVertexData;
    osg::ref_ptr<osg::Vec2Array>   m_bufSizeDistData;
    osg::ref_ptr<osg::Vec4ubArray> m_bufColorData;
    // drawing data
    osg::ref_ptr<osg::DrawArrays>  m_dipDrawArrays;

    // uniform with some settings
    osg::ref_ptr<osg::Uniform> m_uniformSettings;
};

//
// Base visible cultural lights manager class
//

// constructor
PointLightsManager::PointLightsManager()
    : m_nCurrentPass(0)
    , m_pCurrentBulk(NULL)
    , m_fIlluminationCur(1.0f)
    , m_fFogDensityCur(0.0f)
    , m_fVisDistCur(40000.f)
{
    // callbacks
    setUpdateCallback(Utils::makeNodeCallback(this, &PointLightsManager::update));
    setCullCallback(Utils::makeNodeCallback(this, &PointLightsManager::cull));

    // base set up
    setCullingActive(false);
    setDataVariance(osg::Object::STATIC);

    //
    // set-up appropriate state set
    //

    // create it
    osg::StateSet * pLightsStateSet = getOrCreateStateSet();

    // setup render-bin details
    pLightsStateSet->setNestRenderBins(false);
    pLightsStateSet->setRenderBinDetails(RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin");

    // setup blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    pLightsStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

    // enable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
    pLightsStateSet->setAttribute(pDepth);

    // do not write alpha
    osg::ColorMask * pColorMask = new osg::ColorMask(true, true, true, false);
    pLightsStateSet->setAttribute(pColorMask);

    // disable cull-face just for the case
    pLightsStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // disable alpha-to-coverage for that node
    pLightsStateSet->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    // setup texture for point quads
    osg::Texture2D * pLightTex = avCore::GetDatabase()->LoadTexture("light_tex.tga", osg::Texture::CLAMP_TO_EDGE);
    pLightTex->setMaxAnisotropy(4.0f);
    pLightsStateSet->setTextureAttribute(0, pLightTex);

    // setup shader
    osg::Program * pLightProgram = new osg::Program;
    pLightProgram->setName("CulturalLight");
    pLightProgram->addShader(avCore::GetDatabase()->LoadShader("CulturalLight.vs"));
    pLightProgram->addShader(avCore::GetDatabase()->LoadShader("CulturalLight.gs"));
    pLightProgram->addShader(avCore::GetDatabase()->LoadShader("CulturalLight.fs"));
    pLightProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pLightProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pLightProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    // bind shader
    pLightsStateSet->setAttribute(pLightProgram);

    // save local banks dynamic uniforms
    if (avScene::GetScene() && avScene::GetScene()->getWeather())
    {
        avWeather::Weather * pWeatherNode = avScene::GetScene()->getWeather();
        m_pSavedBanksSSMainView = pWeatherNode->GetMainViewLocalFogBanksStateSet();
        m_pSavedBanksSSReflections = pWeatherNode->GetReflectionsLocalFogBanksStateSet();
    }

    // exit
    return;
}

//
// Class methods implementation
//

// function, which is called by users, who want to place visible light
// position must be passed in view space coordinate system
void PointLightsManager::AddVisibleLight(const osg::Vec3f & vViewPos,
                                            const osg::Vec4ub & cColor, float fSize, float fVisDist)
{
    avAssert(m_pCurrentBulk);

    // redirect to currently active collector
    m_pCurrentBulk->addLight(vViewPos, cColor, fSize, fVisDist);
}

// scene call this function according to current weather conditions
void PointLightsManager::SetWeatherConditions(float fFogDensity, float fRealVisDist, float fIllumination)
{
    // just save the data
    m_fFogDensityCur = fFogDensity;
    m_fVisDistCur = fRealVisDist;
    m_fIlluminationCur = fIllumination;
    // exit
    return;
}

//
// OSG callbacks
//

// update pass only resets all counters
void PointLightsManager::update(osg::NodeVisitor * nv)
{
    // flush all the data collected
    for (size_t i = 0; i < m_aPassesBulk.size(); ++i)
        m_aPassesBulk[i]->flushLights();

    // and make current counter equal to zero
    m_nCurrentPass = 0;
    m_pCurrentBulk = NULL;

    // exit
    return;
}

// each succeeding cull pass makes new collector (if not enough) and makes it active, adding it as a drawable
void PointLightsManager::cull(osg::NodeVisitor * nv)
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    // well then, try to make new bulk if needed
    if (m_nCurrentPass == m_aPassesBulk.size())
        m_aPassesBulk.push_back(new CollectedCultLights(this));


    // current bulk is
    m_pCurrentBulk = m_aPassesBulk[m_nCurrentPass].get();

    // save screen clarity
    m_pCurrentBulk->setScreenClarity(Utils::GetScreenClarity(pCV));
    // set reflection pass flag
    osg::Node::NodeMask pCurCVMask = pCV->getCullMask();
    const bool bReflPass = (pCurCVMask == 0x00010000UL);
    m_pCurrentBulk->setReflectionsFlag(bReflPass);

    // extra stateset to be applied
    osg::StateSet * pBanksExtraSS = (bReflPass) ? m_pSavedBanksSSReflections.get() : m_pSavedBanksSSMainView.get();

    // save uniform
    m_pCurrentBulk->saveUniform();

    if (pBanksExtraSS)
        pCV->pushStateSet(pBanksExtraSS);

    // push drawable state set
    pCV->pushStateSet(m_pCurrentBulk->getStateSet());
    // add drawable with collected lights
    pCV->addDrawable(m_pCurrentBulk, NULL);
    // pop drawable state set
    pCV->popStateSet();

    if (pBanksExtraSS)
        pCV->popStateSet();

    // increment current bulk number for a future
    ++m_nCurrentPass;


    // exit
    return;
}
