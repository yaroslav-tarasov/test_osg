#include "stdafx.h"
#include "av/precompiled.h"

#include <osgUtil/UpdateVisitor>
#include <osgUtil/CullVisitor>

#include "utils/callbacks.h"

#include "av/avScene/Scene.h"

#include "Lights.h"

#define LIGHTS_TURN_ON

//
// Module namespaces
//

using namespace avScene;

//
// Helpful lights classes
//

//// light processed info struct
//struct avScene::LightProcessedInfo
//{
//    osg::Vec4f lightVSPosAmbRatio;
//    osg::Vec4f lightVSDirSpecRatio;
//    osg::Vec4f lightAttenuation;
//    osg::Vec3f lightDiffuse;
//};

//// light external info struct
//struct avScene::LightExternalInfo
//{
//    unsigned     uPriority;
//    cg::point_3f vPosWorld;
//    cg::vector_3 vDirWorld;
//    cg::range_2f rDistAtt, rConeAtt;
//    cg::colorf   cDiffuse;
//    float        fAmbRatio, fSpecRatio; 
//
//    bool operator< ( const LightExternalInfo & second ) const
//    {
//        return uPriority < second.uPriority;
//    }
//};




//
// Main lights class
//


/////////////////////////////////////////////////////////////////////
Lights::Lights()
{
    // update visitor is set just to notify parent that we need update traversal
    setUpdateCallback(Utils::makeNodeCallback(this, &Lights::update));

    // base set up
    setCullingActive(false);
    setDataVariance(osg::Object::DYNAMIC);
}

/////////////////////////////////////////////////////////////////////
void Lights::AddLight( LightInfluence dlInfluence, LightType dlType, bool bHighPriority,
                       const cg::point_3f & vWorldPos, const cg::vector_3 & vWorldDir,
                       const cg::range_2f & rDistAtt,const cg::range_2f & rConeAtt,
                       const cg::colorf & cDiffuse, const float & fAmbRatio, const float & fSpecRatio )
{
    LightExternalInfo newLightInfo;
    newLightInfo.uPriority = (unsigned)dlInfluence;
    newLightInfo.vPosWorld = vWorldPos;
    newLightInfo.vDirWorld = vWorldDir;
    newLightInfo.rDistAtt = rDistAtt;
    newLightInfo.rConeAtt = (dlType == ConicalLight) ? rConeAtt : cg::range_2f();
    newLightInfo.cDiffuse = cDiffuse;
    newLightInfo.fAmbRatio = fAmbRatio;
    newLightInfo.fSpecRatio = fSpecRatio;
    newLightInfo.bHighPriority =  bHighPriority;

    m_aFrameActiveLights.push_back(newLightInfo);
}

//
// OSG callbacks
//

// tricky mega-accept
void Lights::accept( osg::NodeVisitor & nv )
{
    // pass down
    switch (nv.getVisitorType())
    {
    case osg::NodeVisitor::UPDATE_VISITOR:
        return update(&nv);
    case osg::NodeVisitor::CULL_VISITOR:
        return cull(&nv);
    }
}


// update pass
void Lights::update( osg::NodeVisitor * nv )
{
    // clear lights
    m_aFrameActiveLights.resize(0);
}

// cull pass
void Lights::cull(osg::NodeVisitor * nv)
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    assert(pCV);

    // current stateset to work with
    const bool bReflPass = (pCV->getCullMask() == REFLECTION_MASK/*0x00010000UL*/);
    std::vector<LightExternalInfo> & aCullVisibleLights = (bReflPass) ? m_aReflVisibleLights : m_aMainVisibleLights;
    std::vector<LightProcessedInfo> & aCullProcessedLights = (bReflPass) ? m_aReflProcessedLights : m_aMainProcessedLights;

    // illumination factor
    FIXME(Sun Intensity студию)
    const float fDarkness = 1.f - (GetScene()->getSky() ? GetScene()->getSky()->GetSunIntensity() : 0.f);
    const float fIllumFactor = cg::lerp01(0.25f, 1.0f,fDarkness * (2.0f - fDarkness));

    // sort lights
    std::sort(m_aFrameActiveLights.begin(), m_aFrameActiveLights.end());

    // decide about visible lights
    const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();
    aCullVisibleLights.resize(0);
    aCullProcessedLights.resize(0);

    // calculate lights for specific pass
    for (unsigned i = 0; i < m_aFrameActiveLights.size(); ++i)
    {
        // XXX: if 0 priority - add always
        FIXME(priority);
        if (m_aFrameActiveLights[i].uPriority > 0)
        {
            // test sphere for visibility
            const cg::point_3f & vWorldPos = m_aFrameActiveLights[i].vPosWorld;
            const cg::range_2f & rDistAtt = m_aFrameActiveLights[i].rDistAtt;
            if (pCV->isCulled(osg::BoundingSphere(osg::Vec3(vWorldPos.x, vWorldPos.y, vWorldPos.z), double(rDistAtt.hi()))))
                continue;
        }

        // add light to visible bulk
        aCullVisibleLights.push_back(m_aFrameActiveLights[i]);
        aCullProcessedLights.push_back(LightProcessedInfo());

        const auto & aCullVisibleLight = aCullVisibleLights.back();
        auto & aCullProcessedLight = aCullProcessedLights.back();
        
        // convert it to uniform-style
        const cg::point_3f & vWorldPos = aCullVisibleLight.vPosWorld;
        const osg::Vec3f vLightVSPos = osg::Vec3(vWorldPos.x, vWorldPos.y, vWorldPos.z) * mWorldToView;
        aCullProcessedLight.lightVSPosAmbRatio = osg::Vec4f(vLightVSPos, aCullVisibleLight.fAmbRatio);
        // convert light direction to view-space
        const cg::vector_3 & vWorldDir = aCullVisibleLight.vDirWorld;
        const osg::Vec3f vLightVSDir = osg::Matrixd::transform3x3(osg::Vec3(vWorldDir.x, vWorldDir.y, vWorldDir.z), mWorldToView);
        aCullProcessedLight.lightVSDirSpecRatio = osg::Vec4f(vLightVSDir, aCullVisibleLight.fSpecRatio);
        // attenuation for distance and for conical angle
        const cg::range_2f & rDistAtt = aCullVisibleLight.rDistAtt;
        const float fDistAttK = rDistAtt.lo() * rDistAtt.hi() / rDistAtt.size();
        const float fDistAttB = - rDistAtt.lo() / rDistAtt.size();
        cg::range_2f rConeAtt = m_aFrameActiveLights[i].rConeAtt;
        if (!rConeAtt.empty())
            rConeAtt = cg::range_2f(-cosf(rConeAtt.lo()), -cosf(rConeAtt.hi()));
        const float fConeAttK = rConeAtt.empty() ? 0.f : (-1.0f / rConeAtt.size());
        const float fConeAttB = rConeAtt.empty() ? 1.f : (rConeAtt.hi() / rConeAtt.size());
        aCullProcessedLight.lightAttenuation = osg::Vec4f(fDistAttK, fDistAttB, fConeAttK, fConeAttB);
        // diffuse color
        const cg::colorf cDiffuse = fIllumFactor * aCullVisibleLight.cDiffuse;
        aCullProcessedLight.lightDiffuse = osg::Vec3f(cDiffuse.r, cDiffuse.g, cDiffuse.b);
    }

    // done!
    return;
}


//
// Then handler, which can be placed in custom cull code
//


// lights states pack structure
LightNodeHandler::LightsPackStateSet::LightsPackStateSet()
{
    // create state-set
    pStateSet = new osg::StateSet();
    pStateSet->setDataVariance(osg::Object::DYNAMIC);
    pStateSet->setNestRenderBins(false);
    // create uniforms array
    LightsActiveNum     = new osg::Uniform(osg::Uniform::INT,        "LightsActiveNum");
    LightVSPosAmbRatio  = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "LightVSPosAmbRatio", nMaxLights);
    LightVSDirSpecRatio = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "LightVSDirSpecRatio", nMaxLights);
    LightAttenuation    = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "LightAttenuation", nMaxLights);
    LightDiffuse        = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "LightDiffuse", nMaxLights);
    // add uniforms
    pStateSet->addUniform(LightsActiveNum.get());
    pStateSet->addUniform(LightVSPosAmbRatio.get());
    pStateSet->addUniform(LightVSDirSpecRatio.get());
    pStateSet->addUniform(LightAttenuation.get());
    pStateSet->addUniform(LightDiffuse.get());
}

bool LightNodeHandler::_init = false;

// constructor
LightNodeHandler::LightNodeHandler( LightInfluence maxInfluenceToUse, bool init )
    : m_pFatherRef(GetScene()?GetScene()->getLights():nullptr)
    , m_uMaxPriority((unsigned)maxInfluenceToUse)
    , m_bStatePushed(false)
    , m_bMainWasActive(false)
    , m_bReflWasActive(false)
{
    _init = init;
    return;
}

// on cull begin
void LightNodeHandler::onCullBegin( osgUtil::CullVisitor * pCV, const osg::BoundingSphere * pSpherePtr /* = NULL */ )
{
    if(!_init)
        return;

    if(!m_pFatherRef)
        m_pFatherRef = GetScene()->getLights();

    // current stateset to work with
    const bool bReflPass = (pCV->getCullMask() == REFLECTION_MASK/*0x00010000UL*/);
    LightsPackStateSet & curStatePack = (bReflPass) ? m_lightsRefl : m_lightsMain;
    const std::vector<LightExternalInfo> & aCullVisibleLights = (bReflPass) ? m_pFatherRef->m_aReflVisibleLights : m_pFatherRef->m_aMainVisibleLights;
    const std::vector<LightProcessedInfo> & aCullProcessedLights = (bReflPass) ? m_pFatherRef->m_aReflProcessedLights : m_pFatherRef->m_aMainProcessedLights;
    // was state pushed last time?
    bool & bWasActive = (bReflPass) ? m_bReflWasActive : m_bMainWasActive;

    unsigned uHighPriorities = std::count_if(aCullVisibleLights.begin(),aCullVisibleLights.end(),[=](const LightExternalInfo& lei){return lei.bHighPriority;});

    // calculate lights for specific geometry
    unsigned uLightsAdded = uHighPriorities;
    unsigned uHighPrioritiesAdded = 0;
    for (unsigned i = 0; i < aCullVisibleLights.size() && uLightsAdded < nMaxLMLights; ++i)
    {
        const LightExternalInfo & lightData = aCullVisibleLights[i];

        // too big priority - continue
        if (lightData.uPriority > m_uMaxPriority)
            continue;

        const LightProcessedInfo & uniformData = aCullProcessedLights[i];

        // calculate whether light touches sphere
        if (pSpherePtr)
        {
            // test for spheres non-intersection
            const osg::Vec3f vVSLightPos(uniformData.lightVSPosAmbRatio.x(), uniformData.lightVSPosAmbRatio.y(), uniformData.lightVSPosAmbRatio.z());
            const float fRadSqrSum = cg::sqr(lightData.rDistAtt.hi() + pSpherePtr->radius());
            const osg::Vec3f vVecToCenter = pSpherePtr->center() - vVSLightPos;
            if (vVecToCenter.length2() >= fRadSqrSum)
                continue;

            // if cone - test for cone intersection
            if (!lightData.rConeAtt.empty() && vVecToCenter.length2() > pSpherePtr->radius2())
            {
                // get direction angle
                osg::Vec3f vVecToCenterDir = vVecToCenter;
                const float fVecLength = vVecToCenterDir.normalize();
                const float
                    fCenterAngle = acosf(vVecToCenterDir * osg::Vec3f(uniformData.lightVSDirSpecRatio.x(), uniformData.lightVSDirSpecRatio.y(), uniformData.lightVSDirSpecRatio.z())),
                    fSphereAngle = atan2f(pSpherePtr->radius(), fVecLength),
                    fMinDirAngle = std::max(fCenterAngle - fSphereAngle, 0.f);
                if (fMinDirAngle >= lightData.rConeAtt.hi())
                    continue;
            }
        }

#ifdef LIGHTS_TURN_ON
        const unsigned uAdded = lightData.bHighPriority? ++uHighPrioritiesAdded:uLightsAdded; 
        
        if(uAdded < nMaxLights)
        {
            // okey, so we can add
            curStatePack.LightVSPosAmbRatio->setElement(uAdded, uniformData.lightVSPosAmbRatio);
            curStatePack.LightVSDirSpecRatio->setElement(uAdded, uniformData.lightVSDirSpecRatio);
            curStatePack.LightAttenuation->setElement(uAdded, uniformData.lightAttenuation);
            curStatePack.LightDiffuse->setElement(uAdded, uniformData.lightDiffuse);
        }
#endif
        uLightsAdded++;
    }

    // so push stateset if needed
    m_bStatePushed = false;
    if (uLightsAdded > 0 || bWasActive)
    {
        curStatePack.LightsActiveNum->set(int(cg::min(uLightsAdded, nMaxLights)));
        pCV->pushStateSet(curStatePack.pStateSet.get());
        bWasActive = (uLightsAdded > 0);
        m_bStatePushed = true;
    }

    // exit
    return;
}

// on cull end
void LightNodeHandler::onCullEnd( osgUtil::CullVisitor * pCV )
{
    if (m_bStatePushed)
        pCV->popStateSet();
    m_bStatePushed = false;
}


//
// At last, convenient cull-method callback
//

// constructor
DynamicLightsObjectCull::DynamicLightsObjectCull( LightInfluence maxInfluenceToUse )
    : m_LightsHandler(maxInfluenceToUse,true)
{
}

// cull callback operator
void DynamicLightsObjectCull::operator()( osg::Node * node, osg::NodeVisitor * nv )
{
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    // get sphere in view-space
    const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();
    osg::BoundingSphere boundSphere = node->getBound();
    boundSphere.center() = boundSphere.center() * mWorldToView;
    

    // cull down
    m_LightsHandler.onCullBegin(pCV, &boundSphere);
    nv->traverse(*node);
    m_LightsHandler.onCullEnd(pCV);
}