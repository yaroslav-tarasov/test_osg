
#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avCore/Utils.h"
#include "av/avCore/Global.h"

#include "av/avScene/Scene.h"
#include "av/avSky/Sky.h"
// #include "svShip/ShipManager.h"

#include "Weather.h"

#include "SnowGlobal.h"
#include "RainGlobal.h"
#include "HailGlobal.h"

#include "FogBank.h"

#include "LocalWeather.h"

//
// Module namespaces
//

using namespace avWeather;


//
// Module functions
//

// static function of getting precipitation color based on its type
osg::Vec3f avWeather::GetPrecipitationColor(PrecipitationType pType)
{
    // precipitations fog dimming
    static const float g_afFogDimFactors[LocalWeatherNumOf] = 
    {
        1.02f, // PrecipitationSnow
        0.73f, // PrecipitationRain
        0.85f, // PrecipitationHail
        1.00f,  // PrecipitationFog
    };

    // scene fog
    const osg::Vec3f & cFogCurrent = avScene::GetScene()->getSky() ? avScene::GetScene()->getSky()->GetFogColor() : osg::Vec3f(1.f, 1.f, 1.f);

    // desaturated fog
    static const osg::Vec3f vDesaturationColor(0.299f, 0.587f, 0.114f);
    const float fFogDesat = cFogCurrent * vDesaturationColor;
    const osg::Vec3f cFogDesaturated(fFogDesat, fFogDesat, fFogDesat);

    // get desaturation factor based on fog multiplier
    const float fDesatFactor = cg::slerp01(cg::lerp_clamp(g_afFogDimFactors[pType], 0.78f, 1.0f, 1.0f, 0.0f));

    // result
    return cg::lerp01(cFogCurrent, cFogDesaturated, fDesatFactor) * g_afFogDimFactors[pType];
}

//////////////////////////////////////////////////////////////////////////

Weather::BanksStateSetPack::BanksStateSetPack()
{
    bankViewToLocal = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "BankViewToLocal", m_nMaxVisibleBanks);
    bankRadiiHeight = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "BankRadiiHeight", m_nMaxVisibleBanks);
    bankRcpRadiiHeight = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "BankRcpRadiiHeight", m_nMaxVisibleBanks);
    bankColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "BankColor", m_nMaxVisibleBanks);
    bankDensityControl = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "BankDensityControl", m_nMaxVisibleBanks);
    // compose them
    ssFogBanks = new osg::StateSet();
    ssFogBanks->setDataVariance(osg::Object::DYNAMIC);
    ssFogBanks->addUniform(bankViewToLocal.get());
    ssFogBanks->addUniform(bankRadiiHeight.get());
    ssFogBanks->addUniform(bankRcpRadiiHeight.get());
    ssFogBanks->addUniform(bankColor.get());
    ssFogBanks->addUniform(bankDensityControl.get());
}

//
// Main weather class
//

// constructor
Weather::Weather()
    : m_fSavedTimeStamp(-1.0f)
    , m_pPrecipitations(PrecipitationNumOf)
{
    // callbacks
    setUpdateCallback(utils::makeNodeCallback(this, &Weather::update));
    setCullCallback(utils::makeNodeCallback(this, &Weather::cull));

    // base set up
    setCullingActive(false);
    setDataVariance(osg::Object::DYNAMIC);

    //
    // set-up appropriate state set
    //

    // create it
    osg::StateSet * pWeatherStateSet = getOrCreateStateSet();

    // setup render-bin details
    pWeatherStateSet->setNestRenderBins(false);
    pWeatherStateSet->setRenderBinDetails(RENDER_BIN_GLOBAL_WEATHER, "RenderBin");
    pWeatherStateSet->setDataVariance(osg::Object::DYNAMIC);

    // setup blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    pWeatherStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

    // enable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
    pWeatherStateSet->setAttribute(pDepth);

    // disable cull-face just for the case
    pWeatherStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // add boolean uniform for precipitations discarding
    m_uniformDepthDiscardingEnabled = new osg::Uniform("DepthDiscardEnabled", bool());
    pWeatherStateSet->addUniform(m_uniformDepthDiscardingEnabled.get());
    // add matrix uniform for precipitations discarding
    m_uniformWorld2ShipDepthTex = new osg::Uniform("World2ShipDepthTex", osg::Matrixf());
    pWeatherStateSet->addUniform(m_uniformWorld2ShipDepthTex.get());
    // and for texture unit
    pWeatherStateSet->addUniform(new osg::Uniform("ShipDepthTex", int(1)));

    //
    // add different precipitations types nodes
    //

    m_pPrecipitations[PrecipitationSnow] = new SnowGlobal();
    m_pPrecipitations[PrecipitationRain] = new RainGlobal();
    m_pPrecipitations[PrecipitationHail] = new HailGlobal();


    // exit
    return;
}


//
// Interface for local weather
//

// remove local weather bank (by ID)
void Weather::RemoveLocalWeatherBank(WeatherBankIdentifier nID)
{
    m_mapLocalBanks.erase(nID);
}

// add or update local weather bank (by ID)
void Weather::UpdateLocalWeatherBank(WeatherBankIdentifier nID,
                            double dLat, double dLon, float fHeading,
                            float fEllipseRadX, float fEllipseRadY, float fHeight,
                            PrecipitationType pType, float fIntensity, float fCentralPortion)
{
    // first - check if presents. if no - create new one
    osg::ref_ptr<LocalWeather> & pBankCur = m_mapLocalBanks[nID];
    if (!pBankCur.valid())
        pBankCur = new LocalWeather(this, size_t(nID));

    // then, change its' data
	FIXME(GeographicPosition);
    //pBankCur->SetGeographicPositionAndRotation(avCore::GeographicCoordinatePosition(dLat, dLon, 0.0), avCore::TaitBryanRotation(fHeading, 0.f, 0.f));
	osg::Matrix mMatrix;
	mMatrix.makeTranslate(osg::Vec3d(dLat, dLon, 0.0));
	pBankCur->SetWorldTransformationMatrix(mMatrix);
	pBankCur->SetPrecipitationDensityPortion(pType, fIntensity, fCentralPortion);
    pBankCur->SetEllipse(fEllipseRadX, fEllipseRadY, fHeight);
}


//
// Internal local weather callbacks
//

// called on culling, so update is already done
void Weather::SetLocalPrecipitationIntensity(PrecipitationType pType, float fLocalInt)
{
    // check only for 3 particles-types precipitations
    if (pType < PrecipitationFog && fLocalInt > 0.0001f)
    {
        // get precipitation
        PrecipitationBase * pPrecipitation = static_cast<PrecipitationBase *>(m_pPrecipitations[pType].get());
        // update intensity
        pPrecipitation->SetIntensity(osg::maximum(pPrecipitation->GetIntensity(), fLocalInt));
    }
}

// get stateset with uniforms for proper fogging
osg::StateSet * Weather::GetMainViewLocalFogBanksStateSet() const
{
    return m_BanksMainView.ssFogBanks.get();
}

// get stateset with uniforms for proper fogging
osg::StateSet * Weather::GetReflectionsLocalFogBanksStateSet() const
{
    return m_BanksRefl.ssFogBanks.get();
}

//
// OSG callbacks
//

// tricky mega-accept
void Weather::accept(osg::NodeVisitor& nv)
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


// update pass only informs about densities and other data
void Weather::update(osg::NodeVisitor * nv)
{
    // get intensity data
    const avCore::Environment::WeatherParameters & cWeatherParameters = avCore::GetEnvironment()->GetWeatherParameters();
    const float faIntensities[PrecipitationNumOf] = 
        { cWeatherParameters.SnowDensity, cWeatherParameters.RainDensity, cWeatherParameters.HailDensity };

    // get wind vector
    const avCore::Environment::EnvironmentParameters & cEnvironmentParameters = avCore::GetEnvironment()->GetEnvironmentParameters();
    
	FIXME(WindDirection);
	//const osg::Vec2f vWindVectorDir(-sinf(cEnvironmentParameters.WindDirection), -cosf(cEnvironmentParameters.WindDirection));
	const osg::Vec2f vWindVectorDir(cEnvironmentParameters.WindDirection.x, cEnvironmentParameters.WindDirection.y);
	
    // inform precipitations about
    for (size_t i = 0; i < PrecipitationNumOf; ++i)
    {
        PrecipitationBase * pPrecipitation = static_cast<PrecipitationBase *>(m_pPrecipitations[i].get());

        pPrecipitation->SetIntensity(faIntensities[i]);
        pPrecipitation->SetWind(vWindVectorDir, cEnvironmentParameters.WindSpeed);
    }

    // get world camera position
    osg::Camera * pSceneCam = avScene::GetScene()->getCamera();
    osg::Vec3f vEye, vCenter, vUp;
    pSceneCam->getViewMatrixAsLookAt(vEye, vCenter, vUp);
	
	FIXME(ship?)
#if 0
    // scan all ships, find closest (but not farther then 30 meters)
    svShip::Ship * pClosest = NULL;
    float fMinDistSqr = osg::square(30.f);
    const size_t nShips = svShip::ShipManager::GetShipsNumber();
    for (size_t i = 0; i < nShips; ++i)
    {
        // get current ship
        svShip::Ship * pShip = svShip::ShipManager::GetShipByIndex(i);

        // convert camera position to ship space
        const osg::Vec3f vEyeShipSpace = vEye * pShip->GetVisualModel()->getInverseMatrix();

        // handle it's bridge
        pShip->HandleCameraRegardingBridge(vEyeShipSpace);
        // get squared distance
        const float fCurDistSqr = avCore::GetDistanceToAABBSqr(vEyeShipSpace, pShip->GetObjectAABB());

        // is it quite close?
        if (fCurDistSqr < fMinDistSqr)
        {
            // save new closest
            fMinDistSqr = fCurDistSqr;
            // save ship
            pClosest = pShip;
            // break if inside
            if (fMinDistSqr == 0.0f)
                break;
        }
    }

    // now if smth is behind us - make discarding to be enabled
    m_uniformDepthDiscardingEnabled->set(bool(pClosest != NULL));
    if (pClosest)
    {
        // calculate world-to-texture transformation and update uniform
        const osg::Matrix mWorldToShip = 
            avCore::GetCoordinateSystem()->GetLTP2LCSMatrix() *
            pClosest->GetVisualModel()->getInverseMatrix() *
            pClosest->GetShipLocalToAABBTexSpace();

        m_uniformWorld2ShipDepthTex->set(mWorldToShip);

        // modify state set
        osg::StateSet * pWeatherStateSet = getOrCreateStateSet();
        // bind texture
        pWeatherStateSet->setTextureAttribute(1, pClosest->GetShipHeightMapDepthTex());
    
	}
#endif

    // exit
    return;
}

// cull pass registers all geometries
void Weather::cull(osg::NodeVisitor * nv)
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
    avAssert(pCV);

    // refl pass?
    const bool bReflPass = (pCV->getCullMask() == 0x00010000UL);
    BanksStateSetPack & bankUniforms = (bReflPass) ? m_BanksRefl : m_BanksMainView;

    // weather params
    const avCore::Environment::WeatherParameters & cWeatherParameters = avCore::GetEnvironment()->GetWeatherParameters();

    // scan all registered local weathers and cull them
    int nLocalBanksVisible = 0;
    LocalBanksMap::iterator bankIt = m_mapLocalBanks.begin();
    while (bankIt != m_mapLocalBanks.end())
    {
        // decide if we skip it
        bool bBankActive = true;
        switch (bankIt->second->GetPrecipitationType())
        {
        case PrecipitationFog:
            bBankActive = cWeatherParameters.LocalFogBanksEnabled;
            break;
        case PrecipitationRain:
            bBankActive = cWeatherParameters.LocalRainBanksEnabled;
            break;
        case PrecipitationSnow:
            bBankActive = cWeatherParameters.LocalSnowBanksEnabled;
            break;
        case PrecipitationHail:
            bBankActive = cWeatherParameters.LocalHailBanksEnabled;
            break;
        }

        // cull bank
        if (bBankActive)
        {
            if (bankIt->second->cull(pCV) && nLocalBanksVisible < m_nMaxVisibleBanks)
            {
#ifdef GOT_FOG
                LocalFogBank * pFogBankCur = bankIt->second->m_pFogBank.get();
                osg::Vec3f vTempVec;
                osg::Vec4f vTempVec4;
                // visible, so add info about that
                bankUniforms.bankViewToLocal->setElement(nLocalBanksVisible, pFogBankCur->m_mViewToLocal);
                pFogBankCur->m_uniformRadiiHeight->get(vTempVec);
                bankUniforms.bankRadiiHeight->setElement(nLocalBanksVisible, vTempVec);
                pFogBankCur->m_uniformRcpRadiiHeight->get(vTempVec);
                bankUniforms.bankRcpRadiiHeight->setElement(nLocalBanksVisible, vTempVec);
                pFogBankCur->m_uniformColor->get(vTempVec);
                bankUniforms.bankColor->setElement(nLocalBanksVisible, vTempVec);
                pFogBankCur->m_uniformDensity->get(vTempVec4);
                bankUniforms.bankDensityControl->setElement(nLocalBanksVisible, vTempVec4);
#endif
                // increment counter
                ++nLocalBanksVisible;
            }
        }

        // go to next one
        ++bankIt;
    }

    // save their count and fill other with zeroes
    avAssert(nLocalBanksVisible <= m_nMaxVisibleBanks);
    for (; nLocalBanksVisible < m_nMaxVisibleBanks; ++nLocalBanksVisible)
        bankUniforms.bankDensityControl->setElement(nLocalBanksVisible, osg::Vec4f());

    // exit if reflections
    if (bReflPass)
        return;


    // first time flag
    const bool
        bFirstTimeFlag = (m_fSavedTimeStamp < 0.f);

    // model-view matrix
    const osg::Matrix
        & mModelView = *pCV->getModelViewMatrix();

    // get camera forward vector and position
    const osg::Vec3f
        vForward(-mModelView(0,2), -mModelView(1,2), -mModelView(2,2)),
        vCamPos(pCV->getEyeLocal());

    // get current time
    const float
        fTimeCur = pCV->getFrameStamp()->getSimulationTime();

    // calculate camera motion vector
    if (bFirstTimeFlag)
    {
        // save history values
        m_vSavedCamPos = vCamPos;
        m_fSavedTimeStamp = fTimeCur;
    }
    else if (!cg::eq(m_fSavedTimeStamp, fTimeCur))
    {
        // update camera velocity
        m_vSavedCamVel = (vCamPos - m_vSavedCamPos) / (fTimeCur - m_fSavedTimeStamp);
        // save history values
        m_vSavedCamPos = vCamPos;
        m_fSavedTimeStamp = fTimeCur;
    }

    // get screen clarity
    const float
        fScrClarity = utils::GetScreenClarity(pCV);
    // get magic illumination
    const float
        fIllum = avScene::GetScene()->getSky() ? avScene::GetScene()->getSky()->GetSunIntensity() : 1.0f,
        fMagicIllum = fIllum * (fIllum * (fIllum - 2.0f) + 2.0f);


    // push
    pCV->pushStateSet(getStateSet());
    // inform about position and register active precipitations
    for (size_t i = 0; i < PrecipitationNumOf; ++i)
    {
        // get one
        PrecipitationBase * pPrecipitation = static_cast<PrecipitationBase *>(m_pPrecipitations[i].get());

        // active?
        // also add it for the first time to make OSG compile shaders and prepare geometries
        if (bFirstTimeFlag || pPrecipitation->IsActive())
        {
            // inform it
            pPrecipitation->SetViewerPos(vCamPos, vForward);
            pPrecipitation->SetCameraVelocity(m_vSavedCamVel);

            // set render data
            pPrecipitation->SetRenderData(fScrClarity, fMagicIllum);

            // register drawable
            _registerPrecipitation(pCV, pPrecipitation);
        }
    }
    // pop
    pCV->popStateSet();

    // exit
    return;
}

//
// Internal functions
//

// for culling pass - add precipitation to rendering bin
void Weather::_registerPrecipitation(osgUtil::CullVisitor * pCV, osg::Geometry * pPrep)
{
    // push drawable state set
    pCV->pushStateSet(pPrep->getStateSet());
    // add drawable with precipitations
    pCV->addDrawable(pPrep, pCV->getModelViewMatrix());
    // pop drawable state set
    pCV->popStateSet();
}
