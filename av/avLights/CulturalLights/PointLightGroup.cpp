
//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avLights/CulturalLights/PointLightsManager.h"
#include "av/avLights/CulturalLights/PointLightGroup.h"

//
// Module namespaces
//

using namespace avScene;
using namespace avLights;

//
// Module constants
//

// this value must be in correspondence with CulturalLight.vp shader
static const float g_fVisDistMultiplier = 2.0f;

//
// Static module methods
//

// clamp angle to appropriate [0; 2*Pi) range
static __forceinline float _angleMakeUnit ( float a )
{
    static const float _2pi = 2.0f * osg::PI;
    a = fmodf(a, _2pi);
    if (a < 0.f)
        a += _2pi;
    return a;
}

// get shortest arc distance between 2 arbitrary angles
static __forceinline float _angularArcDistance ( float a, float b )
{
    static const float _2pi = 2.0f * osg::PI;
    float angle_dist = fmodf(fabsf(b - a), _2pi);
    return __min(angle_dist, _2pi - angle_dist);
}

// get exact distance (counter-clockwise manner) between 2 angles
static __forceinline float _angularCCWDistance ( float a, float b )
{
    static const float _2pi = 2.0f * osg::PI;
    return (b > a) ? (b - a) : (_2pi - (a - b));
}

//
// Base point light group class
//

// constructor
PointLightGroup::PointLightGroup()
    : m_fMaxVisDistSqr(0.0f)
    , m_fThresholdDelta(0.0f)
{
    // base set up
    setDataVariance(osg::Object::STATIC);
    setCullingActive(false);

    // exit
    return;
}

//
// Interface methods implementation
//


// base set up: clear all lights
void PointLightGroup::ClearLights()
{
    m_aCultLights.resize(0);
    m_aNavLights.resize(0);
    m_aabbLights = osg::BoundingBox();
    m_fMaxVisDistSqr = 0.0f;
}

// base set up: add light
void PointLightGroup::AddCulturalLight( const osg::Vec3f & vPos, const osg::Vec4ub & cCol, float fSize, float fVisDist )
{
    // new light
    CultLightInfo newLight;
    newLight.vPos = vPos;
    newLight.cCol = cCol;
    newLight.fSize = fSize;
    newLight.fVisDist = fVisDist;

    // add it
    m_aCultLights.push_back(newLight);

    // expand BB
    m_aabbLights.expandBy(vPos);

    // update max distance
    m_fMaxVisDistSqr = osg::maximum(m_fMaxVisDistSqr, osg::square(g_fVisDistMultiplier * fVisDist));
}

// base set up: add light
void PointLightGroup::AddNavigationalLight( const osg::Vec3f & vPos, const osg::Vec4ub & cCol, float fSize, float fVisDist,
                                            float fStartAngle, float fFinishAngle, const std::vector<float> * paBlinkSeq, float fTimeDelta )
{
    // new light
    NavLightInfo newLight;
    newLight.vPos = vPos;
    newLight.cCol = cCol;
    newLight.fSize = fSize;
    newLight.fVisDist = fVisDist;
    newLight.fSectorStart = _angleMakeUnit(fStartAngle);
    newLight.fSectorEnd = _angleMakeUnit(fFinishAngle);
    if ((newLight.fSectorStart + newLight.fSectorEnd) >= 0.0001f)
    {
        const float fSectorSize = _angularCCWDistance(newLight.fSectorStart, newLight.fSectorEnd);
        newLight.fSectorFadeInv = osg::minimum(osg::DegreesToRadians(3.0f), 0.2f * fSectorSize);
        newLight.fSectorFadeInv = 1.0f / osg::maximum(newLight.fSectorFadeInv, 0.0001f);
    }
    else
        newLight.fSectorFadeInv = 0.f;
    newLight.fTimeDelta = fTimeDelta;
    if (paBlinkSeq == NULL || paBlinkSeq->empty())
        newLight.fSequenceDuration = -1.f;
    else
    {
        newLight.fSequenceDuration = 0.f;
        newLight.aBlinkSequenceData.resize(paBlinkSeq->size() >> 1);
        for (size_t i = 0; i < newLight.aBlinkSequenceData.size(); ++i)
        {
            osg::Vec4f & vBlinkData = newLight.aBlinkSequenceData[i];
            const float & fOnTime = paBlinkSeq->at(i * 2 + 0);
            newLight.fSequenceDuration += fOnTime;
            vBlinkData[0] = newLight.fSequenceDuration;
            const float & fOffTime = paBlinkSeq->at(i * 2 + 1);
            newLight.fSequenceDuration += fOffTime;
            vBlinkData[1] = newLight.fSequenceDuration;
            // fill falloff phase for smoothing light appearance
            const float fFadeTime = osg::minimum(0.1f, 0.1f * fOffTime);
            vBlinkData[2] = osg::maximum(0.0001f, fFadeTime);
            vBlinkData[3] = 1.0f / vBlinkData[2];
        }
    }

    // add it
    m_aNavLights.push_back(newLight);

    // expand BB
    m_aabbLights.expandBy(vPos);

    // update max distance
    m_fMaxVisDistSqr = osg::maximum(m_fMaxVisDistSqr, osg::square(g_fVisDistMultiplier * fVisDist));
}

// base set up: enable threshold
void PointLightGroup::SetEnableThreshold( float fThreshold )
{
    m_fThresholdDelta = fThreshold;
}

// get info: get lights number
size_t PointLightGroup::GetLightsNum() const
{
    return m_aCultLights.size() + m_aNavLights.size();
}

//
// OSG accept() tricky overload
//

// we make accept() and call appropriate callbacks by ourselves
void PointLightGroup::accept(osg::NodeVisitor & nv)
{
    // pass down
    switch (nv.getVisitorType())
    {
    case osg::NodeVisitor::CULL_VISITOR:
        return cull(&nv);
    }
}


//
// OSG callbacks
//

// decide about sectoring and add light if visible
void PointLightGroup::cull(osg::NodeVisitor * nv)
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    // look onto environment weather thresholds
    const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();

    // flags
    const bool bCultLightsEnabled = (cIlluminationParameters.Illumination <= cg::clamp01(cIlluminationParameters.CulturalLightsLightsThreshold + m_fThresholdDelta));
    const bool bNaviLightsEnabled = (cIlluminationParameters.Illumination <= cg::clamp01(cIlluminationParameters.NavigationLightsThreshold + m_fThresholdDelta));

    // decide about cull state
    if ((!bCultLightsEnabled && !bNaviLightsEnabled) || pCV->isCulled(m_aabbLights))
        return;
    // decide about distance clip (too far for the most strong light)
    if (Utils::GetDistanceToAABBSqr(nv->getEyePoint(), m_aabbLights) >= m_fMaxVisDistSqr)
        return;

    // obtain manager pointer
    PointLightsManager * pCLM = Scene::GetInstance()->getPointLightsManager();

    // convert point to view space
    const osg::Matrix & mModelView = *pCV->getModelViewMatrix();

    // cultural lights
    if (bCultLightsEnabled)
    {
        // add lights in a loop
        for (size_t i = 0; i < m_aCultLights.size(); ++i)
        {
            const CultLightInfo & curLight = m_aCultLights[i];

            pCLM->AddVisibleLight(curLight.vPos * mModelView, curLight.cCol, curLight.fSize, curLight.fVisDist);
        }
    }

    // navigational lights
    if (bNaviLightsEnabled)
    {
        // get time
        const float fSimTime = nv->getFrameStamp()->getSimulationTime();

        // add lights in a loop
        for (size_t i = 0; i < m_aNavLights.size(); ++i)
        {
            const NavLightInfo & curLight = m_aNavLights[i];

            // test for blinking phase, if needed
            float fBlinkAlpha = 1.0f;
            if (curLight.fSequenceDuration > 0.0f)
            {
                // get local time
                const float fLocalTime = fmodf(fSimTime + curLight.fTimeDelta, curLight.fSequenceDuration);
                // test even and odd numbers
                for (size_t i = 0; i < curLight.aBlinkSequenceData.size(); ++i)
                {
                    const osg::Vec4f & vBlinkData = curLight.aBlinkSequenceData[i];
                    // test for "on" phase timing match
                    if (fLocalTime <= vBlinkData[0])
                        break;
                    // may be, off phase?
                    if (fLocalTime < vBlinkData[1])
                        // calculate blinking force
                        fBlinkAlpha = 1.f - osg::minimum(fLocalTime - vBlinkData[0], vBlinkData[1] - fLocalTime) * vBlinkData[3];
                }
            }
            // early exit if disabled
            if (fBlinkAlpha <= 0.f)
                continue;

            // view pos
            const osg::Vec3f vViewPos = curLight.vPos * mModelView;
            if (vViewPos.z() >= 0.f)
                continue;

            // sectoring impact
            bool bSectorPassed = true;
            float fSectorFading = 1.f;
            // test for sectoring, if needed
            if (curLight.fSectorFadeInv > 0.0f)
            {
                // get X,Y directions in a view space
                const osg::Vec3f vX(mModelView(0, 0), mModelView(0, 1), mModelView(0, 2));
                const osg::Vec3f vY(mModelView(1, 0), mModelView(1, 1), mModelView(1, 2));
                // convert to a planar point
                const osg::Vec2f vPlanarDir(-(vX * vViewPos), -(vY * vViewPos));

                // get angle in [0..2Pi] range
                float fAngleRad = atan2f(vPlanarDir.x(), vPlanarDir.y());
                if (fAngleRad < 0.f)
                    fAngleRad += 2.0f * osg::PI;
                // test for sector
                bSectorPassed = (curLight.fSectorStart < curLight.fSectorEnd) ?
                    ((fAngleRad > curLight.fSectorStart) && (fAngleRad < curLight.fSectorEnd)) :
                ((fAngleRad > curLight.fSectorStart) || (fAngleRad < curLight.fSectorEnd));
                // may be, fading?
                if (!bSectorPassed)
                {
                    fSectorFading = 1.f - curLight.fSectorFadeInv * osg::minimum(_angularArcDistance(fAngleRad, curLight.fSectorStart), _angularArcDistance(fAngleRad, curLight.fSectorEnd));
                    fSectorFading = osg::clampAbove(fSectorFading, 0.f);
                }
            }

            // add light
            if (fSectorFading > 0.f)
            {
                // result fading
                const float fResultFading = fSectorFading * fBlinkAlpha;
                // fade color
                osg::Vec4ub cFadedColor = curLight.cCol;
                cFadedColor.a() *= fResultFading;
                // add light
                pCLM->AddVisibleLight(vViewPos, cFadedColor, curLight.fSize * fResultFading, curLight.fVisDist);
            }
        }
    }

    // exit
    return;
}
