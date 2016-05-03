//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avLights/CulturalLights/PointLightsManager.h"
#include "av/avLights/CulturalLights/NavigationalLight.h"

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
// Base navigational light class
//

// constructor
NavigationalLight::NavigationalLight()
    : m_exhCondition(NightTimeOnly)
    , m_bEnabled(true)
    , m_fSize(1.0f)
    , m_fVisDist(18525.0f)
    , m_fVisDistMaxSqr(osg::square(g_fVisDistMultiplier * m_fVisDist))
    , m_cColor(0xFF, 0x00, 0x00, 0xFF)
    , m_fSectorStart(0.0f)
    , m_fSectorEnd(0.0f)
    , m_fSectorFadeInv(0.f)
    , m_fTimeDelta(0.f)
    , m_fSequenceDuration(-1.0f)
    , m_nCachedFrameNumber(-1)
    , m_fBlinkAlpha(1.0f)
{
    // callbacks
    // update visitor is set just to notify parent that we need update traversal
    //setUpdateCallback(Utils::makeNodeCallback(this, &NavigationalLight::update));
    //setCullCallback(Utils::makeNodeCallback(this, &NavigationalLight::cull));

    // base set up
    setDataVariance(osg::Object::STATIC);
    setCullingActive(false);

    // bound
    //setInitialBound(osg::BoundingSphere(m_vLocalPos, m_fSize));

    // exit
    return;
}

//
// Interface methods implementation
//


// base set up: enable state
void NavigationalLight::SetEnableState(bool bOn)
{
    m_bEnabled = bOn;
}

// base set up: enable threshold using
void NavigationalLight::SetExhibitionCondition(LightExhibitionCondition condition)
{
    m_exhCondition = condition;
}

// base set up: local position
void NavigationalLight::SetLocalPosition(const osg::Vec3f & vPos)
{
    m_vLocalPos = vPos;
    // bound
    //setInitialBound(osg::BoundingSphere(m_vLocalPos, m_fSize));
}

// base set up: size
void NavigationalLight::SetSize(float fSize)
{
    m_fSize = fSize;
    // bound
    //setInitialBound(osg::BoundingSphere(m_vLocalPos, m_fSize));
}

// base set-up: visible distance for light
void NavigationalLight::SetVisDistance(float fVisDist)
{
    // clamp with 20 nautical miles
    m_fVisDist = osg::minimum(fVisDist, 37050.0f);
    m_fVisDistMaxSqr = osg::square(g_fVisDistMultiplier * m_fVisDist);
}

// base set up: color
void NavigationalLight::SetColor(const osg::Vec4ub & cCol)
{
    m_cColor = cCol;
}


// sector set up: sector size
void NavigationalLight::SetSectorRange(float fStartAngle, float fFinishAngle)
{
    // save sector start and finish
    m_fSectorStart = _angleMakeUnit(fStartAngle);
    m_fSectorEnd = _angleMakeUnit(fFinishAngle);

    // set sector fading
    if ((m_fSectorStart + m_fSectorEnd) >= 0.0001f)
    {
        // sector size
        const float fSectorSize = _angularCCWDistance(m_fSectorStart, m_fSectorEnd);
        // get angle of fading as minimum between 3.0f degrees and fSectorSize/5
        m_fSectorFadeInv = osg::minimum(osg::DegreesToRadians(3.0f), 0.2f * fSectorSize);
        // save its reciprocal
        m_fSectorFadeInv = 1.0f / osg::maximum(m_fSectorFadeInv, 0.0001f);
    }
    else
        // no sectoring
        m_fSectorFadeInv = 0.f;
}


// blinking set-up: blinking
void NavigationalLight::SetBlinkingSequence(const std::vector<float> & aBlinkSeq)
{
    // test for emptying blinking sequence
    if (aBlinkSeq.empty())
    {
        // empty sequence case
        m_aBlinkSequenceData.resize(0);
        m_fSequenceDuration = -1.f;
        // release from update callback
        m_fBlinkAlpha = 1.0f;
        setUpdateCallback(NULL);
        // exit
        return;
    }

    // ensure sequence size is even number
    avAssert((aBlinkSeq.size() & 1) == 0);

    // make appropriate blink data vector
    m_fSequenceDuration = 0.f;
    m_aBlinkSequenceData.resize(aBlinkSeq.size() >> 1);
    for (size_t i = 0; i < m_aBlinkSequenceData.size(); ++i)
    {
        osg::Vec4f & vBlinkData = m_aBlinkSequenceData[i];
        // lit phase
        const float & fOnTime = aBlinkSeq[i * 2 + 0];
        m_fSequenceDuration += fOnTime;
        vBlinkData[0] = m_fSequenceDuration;
        // off phase
        const float & fOffTime = aBlinkSeq[i * 2 + 1];
        m_fSequenceDuration += fOffTime;
        vBlinkData[1] = m_fSequenceDuration;
        // fill falloff phase for smoothing light appearance
        const float fFadeTime = osg::minimum(0.1f, 0.1f * fOffTime);
        vBlinkData[2] = osg::maximum(0.0001f, fFadeTime);
        vBlinkData[3] = 1.0f / vBlinkData[2];
    }

    // make update callback if needed
    if (!getUpdateCallback())
        setUpdateCallback(Utils::makeNodeCallback(this, &NavigationalLight::update));
}

// blinking set-up: delay
void NavigationalLight::SetBlinkingDelay(float fDelay)
{
    m_fTimeDelta = fDelay;
}

// getters: enable state
const bool NavigationalLight::GetEnableState() const
{
    // early exit if disabled
    if (!m_bEnabled)
        return false;

    // look onto environment weather thresholds
    const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
    return ((m_exhCondition != NightTimeOnly || cIlluminationParameters.Illumination < cIlluminationParameters.NavigationLightsThreshold) &&
            (m_exhCondition != DayTimeOnly || cIlluminationParameters.Illumination > cIlluminationParameters.NavigationLightsThreshold));
}

// getters: position
const osg::Vec3f & NavigationalLight::GetLocalPosition() const
{
    return m_vLocalPos;
}

// getters: color
const osg::Vec4ub & NavigationalLight::GetColor() const
{
    return m_cColor;
}

//
// OSG accept() tricky overload
//

// we make accept() and call appropriate callbacks by ourselves
void NavigationalLight::accept(osg::NodeVisitor & nv)
{
    // early exit if disabled
    if (!GetEnableState())
        return;

    // pass down
    switch (nv.getVisitorType())
    {
    case osg::NodeVisitor::UPDATE_VISITOR:
        return update(&nv);
    case osg::NodeVisitor::CULL_VISITOR:
        return cull(&nv);
    }
}


//
// OSG callbacks
//

// update pass gets the info about what to do when blinking and so on
void NavigationalLight::update(osg::NodeVisitor * nv)
{
    // update pass
    osgUtil::UpdateVisitor * pUV = static_cast<osgUtil::UpdateVisitor *>(nv);
    avAssert(pUV);

    // get frame stamp
    const osg::FrameStamp * pFrameStamp = nv->getFrameStamp();
    // early exit if cached done
    if (pFrameStamp->getFrameNumber() == m_nCachedFrameNumber)
        return;

    // save frame number
    m_nCachedFrameNumber = pFrameStamp->getFrameNumber();

    // test for blinking phase, if needed
    m_fBlinkAlpha = 1.0f;
    if (m_fSequenceDuration > 0.0f)
    {
        // get local time
        const float fLocalTime = fmodf(pFrameStamp->getSimulationTime() + m_fTimeDelta, m_fSequenceDuration);
        // test even and odd numbers
        for (size_t i = 0; i < m_aBlinkSequenceData.size(); ++i)
        {
            const osg::Vec4f & vBlinkData = m_aBlinkSequenceData[i];
            // test for "on" phase timing match
            if (fLocalTime <= vBlinkData[0])
                break;
            // may be, off phase?
            if (fLocalTime < vBlinkData[1])
                // calculate blinking force
                m_fBlinkAlpha = 1.f - osg::minimum(fLocalTime - vBlinkData[0], vBlinkData[1] - fLocalTime) * vBlinkData[3];
        }
    }

    // exit
    return;
}

// decide about sectoring and add light if visible
void NavigationalLight::cull(osg::NodeVisitor * nv)
{
    // early exit if disabled
    if (m_fBlinkAlpha <= 0.f)
        return;

    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    // convert point to view space
    const osg::Matrix & mModelView = *pCV->getModelViewMatrix();
    const osg::Vec3f vViewPos = m_vLocalPos * mModelView;

    // early rejection based on distance or back-side
    if ((vViewPos.z() >= 0.f) || (vViewPos * vViewPos > m_fVisDistMaxSqr))
        return;

    // sectoring impact
    bool bSectorPassed = true;
    float fSectorFading = 1.f;

    // test for sectoring, if needed
    if (m_fSectorFadeInv > 0.0f)
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
        bSectorPassed = (m_fSectorStart < m_fSectorEnd) ?
            ((fAngleRad > m_fSectorStart) && (fAngleRad < m_fSectorEnd)) :
            ((fAngleRad > m_fSectorStart) || (fAngleRad < m_fSectorEnd));

        // may be, fading?
        if (!bSectorPassed)
        {
            fSectorFading = 1.f - m_fSectorFadeInv *
                osg::minimum(_angularArcDistance(fAngleRad, m_fSectorStart), _angularArcDistance(fAngleRad, m_fSectorEnd));
            fSectorFading = osg::clampAbove(fSectorFading, 0.f);
        }
    }

    // add light
    if (fSectorFading > 0.f)
    {
        // obtain manager pointer
        PointLightsManager * pCLM = Scene::GetInstance()->getPointLightsManager();
        // result fading
        const float fResultFading = fSectorFading * m_fBlinkAlpha;
        // fade color
        osg::Vec4ub cFadedColor = m_cColor;
        cFadedColor.a() *= fResultFading;
        // add light point fro latter drawing
        pCLM->AddVisibleLight(vViewPos, cFadedColor, m_fSize * fResultFading, m_fVisDist);
    }

    // exit
    return;
}
