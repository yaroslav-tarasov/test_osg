#include <stdafx.h>

#include <osgUtil/CullVisitor>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avWeather/FogBank.h"
#include "av/avWeather/Cloud.h"

#include "av/avWeather/LocalWeather.h"

//
// Module namespaces
//

using namespace avWeather;



//
// Local weather bank class
// Implements cylindrical local weather functionality
//

// constructor
LocalWeather::LocalWeather(Weather * pFather, size_t nID)
    : m_pFatherRef(pFather)
    , m_fRadX(2000.f)
    , m_fRadY(500.f)
    , m_fMaxH(500.f)
    , m_ptType(PrecipitationFog)
    , m_fDensity(1.0f)
    , m_fPortion(0.5f)
{
    // fog bank object
    m_pFogBank = new LocalFogBank();

    // cloud
    m_pCloud = new Cloud(nID);

    // exit
    return;
}

//
// Base local weather interface
//

// set bank radii and height (min height is assuming as tide level, which is zero for Environment node, under which we are placed)
void LocalWeather::SetEllipse(float fRadX, float fRadY, float fMaxH)
{
    // save new values
    m_fRadX = osg::maximum(fRadX, 100.f);
    m_fRadY = osg::maximum(fRadY, 100.f);
    m_fMaxH = osg::maximum(fMaxH, 25.f);

    // calculate cloud params
    const float fCloudVerticalSize = 0.33f * (m_fRadX + m_fRadY) + 0.40f * m_fMaxH;
    // tell them to fog bank
    m_pCloud->SetEllipsoid(m_fRadX, m_fRadY, fCloudVerticalSize, m_fMaxH);

    // tell them to fog bank
    const float fFogHeightDelta = (m_ptType == PrecipitationFog) ? 0.f : 0.5f * m_pCloud->GetHeightCorrection();
    m_pFogBank->SetEllipse(m_fRadX, m_fRadY, m_fMaxH + fFogHeightDelta);

}

// set precipitation, density and portion
void LocalWeather::SetPrecipitationDensityPortion(PrecipitationType pType, float fInnerInt, float fPortion)
{
    // save new values
    m_ptType = pType;
    m_fDensity = cg::clamp01(fInnerInt);
    m_fPortion = osg::minimum(cg::clamp01(fPortion), 0.999999f);

    // calculate appropriate densities and so on
    // snow/rain/hail have decreasing densities toward sea/ground
    // fog, in opposite, decreases in density toward sky
    float
        fFogDensity = m_fDensity * (m_fDensity * (m_fDensity - 2.0f) + 2.0f),
        fFogDownScale = 1.0f;
    switch (m_ptType)
    {
    case PrecipitationSnow:
        fFogDensity = Utils::ConvertTraineeDensityToExpSceneFog(0.65f * fFogDensity);
        fFogDownScale = cg::lerp_clamp(m_fMaxH, 50.f, 500.f, 0.90f, 0.20f);
        break;
    case PrecipitationRain:
        fFogDensity = Utils::ConvertTraineeDensityToExpSceneFog(0.60f * fFogDensity);
        fFogDownScale = cg::lerp_clamp(m_fMaxH, 50.f, 500.f, 0.90f, 0.20f);
        break;
    case PrecipitationHail:
        fFogDensity = Utils::ConvertTraineeDensityToExpSceneFog(0.55f * fFogDensity);
        fFogDownScale = cg::lerp_clamp(m_fMaxH, 50.f, 500.f, 0.90f, 0.20f);
        break;
    case PrecipitationFog:
        fFogDownScale = cg::lerp_clamp(m_fMaxH, 20.f, 200.f, 1.0f, 10.0f);
        fFogDensity = Utils::ConvertTraineeDensityToExpSceneFog(m_fDensity) / fFogDownScale;
        break;
    }

    // pass them to fog bank
    m_pFogBank->SetPrecipitationDensityPortion(m_ptType, fFogDensity, cg::lerp01(osg::square(m_fPortion), 0.25f, 0.75f), fFogDownScale);

    // pass also to cloud
    m_pCloud->SetPrecipitationDensity(m_ptType, m_fDensity);

    // exit
    return;
}


//
// Transformation interface
//

// react on matrix has been changed
void LocalWeather::SetWorldTransformationMatrix(const osg::Matrix & mMatrix)
{
    // save model matrix as model->LTP
    m_mModelMatrix = mMatrix /** avCore::GetCoordinateSystem()->GetLCS2LTPMatrix()*/;
}


//
// Special culling
//

// special culling method
// return value - is local fog part visible
bool LocalWeather::cull(osgUtil::CullVisitor * pCV)
{
    // refl pass?
    const bool bReflPass = (pCV->getCullMask() == REFLECTION_MASK/*0x00010000UL*/);

    // push matrix
    osg::ref_ptr<osg::RefMatrix> refModelViewMatrix = new osg::RefMatrix(*pCV->getModelViewMatrix());
    refModelViewMatrix->preMult(m_mModelMatrix);
    pCV->pushModelViewMatrix(refModelViewMatrix.get(), osg::Transform::RELATIVE_RF);

    // local cloud on top may be visible
    if (m_ptType != PrecipitationFog && !pCV->isCulled(m_pCloud->GetEllipsoidAABB()))
    {
        // ok so make culling of local fog part
        m_pCloud->cull(pCV);
    }

    // is visible or inside region somehow?

    bool bFogVisible = false;
    if (!pCV->isCulled(m_pFogBank->GetCyliderAABB()))
    {
        // ok so make culling of local fog part
        m_pFogBank->cull(pCV);
        bFogVisible = true;

        // and decide whether we are inside
        if (!bReflPass)
        {
            osg::Vec3f vCamPos(pCV->getEyeLocal());
            vCamPos.z() = osg::maximum(0.f, vCamPos.z());
            if (m_pFogBank->GetCyliderAABB().contains(vCamPos))
            {
                // calculate intensity
                const osg::Vec2f vUnitVec(vCamPos.x() / m_fRadX, vCamPos.y() / m_fRadY);
                const float fLinerDensity = cg::lerp_clamp(vUnitVec.length(), m_fPortion, 1.0f, 1.0f, 0.f);
                m_pFatherRef->SetLocalPrecipitationIntensity(m_ptType, m_fDensity * osg::square(fLinerDensity));
            }
        }
    }

    // pop model view matrix
    pCV->popModelViewMatrix();

    // exit
    return bFogVisible;
}
