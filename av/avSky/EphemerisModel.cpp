#include "stdafx.h"

#include "av/precompiled.h"

#include "av/avScene/Scene.h"

#include "EphemerisModel.h"

#include "av/CloudLayer.h"

//
// Global namespaces
//


using namespace avSky;


EphemerisModel::EphemerisModel(const EphemerisModel& copy, const osg::CopyOp& copyop )
    : _needsToUpdate(true)
{
}

EphemerisModel::EphemerisModel( osg::Group * scene )
    : _illumination(1.0f)
    , _scene(scene)
    , _needsToUpdate(true)
    , _nCloudsType(-1)
    , _fCloudsDensity(0.0f)
    , _fogDensity(0.f)
    , _cFogColor(1.0f, 1.0f, 1.0f)
    , _manualSunElevationMode(false)
    , _desiredIllumination(1.0f)
{
    avAssert( _scene );

    // explicitly make all data
    _ephemerisEngine = new EphemerisEngine(this);

    // set our update callback
    setUpdateCallback(utils::makeNodeCallback(this, &EphemerisModel::update));
   
    setCullCallback(utils::makeNodeCallback(this, &EphemerisModel::cull, true));


    // create skydome, etc
    _initialize();

    // set default settings
    _setDefault();

    
}


bool EphemerisModel::_initialize()
{
    avAssert(_scene);

    // add sun light-source
    _sunLightSource = new osg::LightSource();
    _sunLightSource->getLight()->setLightNum(0);
    _scene->addChild(_sunLightSource.get());

    // sky dome child
    _skyDome = new SkyDome();
    addChild(_skyDome.get());

    // fog layer
    _skyFogLayer = new avSky::FogLayer(_scene);
    addChild(_skyFogLayer.get());

    // moon
    _skyMoon = new MoonSphere();
    addChild(_skyMoon.get());

    // star field
    _skyStarField = new StarField();
    addChild(_skyStarField.get());

    // clouds

    // Create clouds
    _skyClouds = new avSky::CloudsLayer(_scene);
    addChild(_skyClouds.get());

     osg::StateSet * pSceneSS = _scene->getOrCreateStateSet();

    // illumination uniform
    _uniformSunIntensity = new osg::Uniform("sunIntensity", float(1.0f));
    pSceneSS->addUniform(_uniformSunIntensity.get());

    _ambientUniform = new osg::Uniform("ambient", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_ambientUniform.get());

    _diffuseUniform = new osg::Uniform("diffuse", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_diffuseUniform.get());

    // FIXME TODO secular.a wanna rain
    _specularUniform = new osg::Uniform("specular", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_specularUniform.get());

    _lightDirUniform = new osg::Uniform("light_vec_view", osg::Vec4f(0.f, 0.f, 0.f, 1.f));
    pSceneSS->addUniform(_lightDirUniform.get());
    
    return true;
}


void EphemerisModel::setLatitude( double latitude)
{
    _ephemerisEngine->setLatitude(latitude);
}

double EphemerisModel::getLatitude() const
{
    return latitude;
}

double EphemerisModel::getLongitude() const
{
    return longitude;
}

void EphemerisModel::setLongitude( double longitude)
{
    _ephemerisEngine->setLongitude(longitude);
}

void EphemerisModel::setLatitudeLongitude( double latitude, double longitude )
{
    _needsToUpdate |= _ephemerisEngine->setLatitudeLongitude(latitude, longitude);
}

void EphemerisModel::getLatitudeLongitude( double & out_latitude, double & out_longitude ) const
{
    out_latitude = latitude;
    out_longitude = longitude;
}

void EphemerisModel::setLatitudeLongitudeAltitude( double latitude, double longitude, double altitude )
{
    _needsToUpdate |= _ephemerisEngine->setLatitudeLongitudeAltitude(latitude, longitude, altitude);
}

void EphemerisModel::getLatitudeLongitudeAltitude( double & out_latitude, double & out_longitude, double & out_altitude ) const
{
    out_latitude = latitude;
    out_longitude = longitude;
    out_altitude = altitude;
}

void EphemerisModel::setDateTime( const DateTime & dateTime )
{
    _needsToUpdate |= _ephemerisEngine->setDateTime(dateTime);
}

DateTime EphemerisModel::getDateTime() const
{
    return dateTime;
}

void EphemerisModel::setHumidityTemperature( float humidity, float skytemperature )
{
    _needsToUpdate |= _ephemerisEngine->setTurbidityTemperature(powf(humidity, 0.95f) * 30.0f, skytemperature);
    _skyDome->setTurbidity(turbidity);
    _skyDome->setTemperature(temperature);
}

// cull method
void EphemerisModel::cull( osg::NodeVisitor * pNV )
{
    // get cull visitor
    osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
    avAssert(pCV);

    _mv = *pCV->getModelViewMatrix();

    // cull down
    
    FIXME("Хорошее место, но есть лучше");
    PreUpdate();
    pNV->traverse(*this);

}

void EphemerisModel::update( osg::NodeVisitor * nv )
{
    if (_needsToUpdate)
    {
        // update Ephemeris engine
        _ephemerisEngine->update();

        // update sun
        _updateSun();
        // update moon
        _updateMoon();
        // update stars
        _updateStars();

        _updateClouds();
        
        // update sky
        _skyDome->updateSkyTexture();

        // calculate illumination factors
        _calculateIllumFactors();

        // get vis distance
        const float fVisDist = _skyFogLayer->getVisDistance();

        FIXME("Cult lights");
        // inform cultural lights manager about new values
        // avScene::Scene::GetInstance()->getPointLightsManager()->SetWeatherConditions(_fogDensity, fVisDist, _illumination);

        // all done
        _needsToUpdate = false;
    }
}

void EphemerisModel::_updateSun()
{
    // get sun azimuth from osgEphemeris data
    float sunAz = osg::RadiansToDegrees(sunAzimuth);

    // get sun elevation
    float sunAlt = 90.0f;
    if (_manualSunElevationMode)
    {
        // adopt somehow illumination to sun elevation
        if (_desiredIllumination <= 0.10f)
            sunAlt = cg::lerp_clamp(_desiredIllumination, 0.00f, 0.10f, -15.f, -12.5f);
        else if (_desiredIllumination <= 0.25f)
            sunAlt = cg::lerp_clamp(_desiredIllumination, 0.10f, 0.25f, -12.5f, -08.0f);
        else if (_desiredIllumination <= 0.35f)
            sunAlt = cg::lerp_clamp(_desiredIllumination, 0.25f, 0.35f, -08.0f, -05.0f);
        else if (_desiredIllumination <= 0.60f)
            sunAlt = cg::lerp_clamp(_desiredIllumination, 0.35f, 0.60f, -05.0f, +01.5f);
        else if (_desiredIllumination <= 0.85f)
            sunAlt = cg::lerp_clamp(_desiredIllumination, 0.60f, 0.85f, +01.5f, +18.0f);
        else
            sunAlt = cg::lerp_clamp(_desiredIllumination, 0.85f, 1.00f, +18.f, +45.f);
    }
    else
    {
        // get it directly from ephemeris
        sunAlt = osg::RadiansToDegrees(sunAltitude);
    }

    // tell sky-dome where sun is
    _skyDome->setSunPos(sunAz, sunAlt);
}

void EphemerisModel::_updateMoon()
{
    if (_skyMoon.valid())
    {
        _skyMoon->setDirection(moonAzimuth, moonAltitude);
    }
}

void EphemerisModel::_updateStars()
{
    if(_skyStarField.valid())
    {
        _skyStarField->setMatrix(
            osg::Matrix::rotate( -(-1.0 + (localSiderealTime/12.0)) * osg::PI, osg::Vec3(0, 0, 1)) *
            osg::Matrix::rotate( -osg::DegreesToRadians((90.0 - latitude)), osg::Vec3(1, 0, 0))
        );
    }
}

void EphemerisModel::_updateClouds()
{
    if(_skyClouds.valid())
    {
        const float fCloudLum = cg::max(0.06f, _illumination);
        _skyClouds->setCloudsColors(osg::Vec3f(fCloudLum,fCloudLum,fCloudLum), osg::Vec3f(fCloudLum,fCloudLum,fCloudLum));

        _skyClouds->setRotationSiderealTime(-float(fmod(localSiderealTime / 24.0, 1.0)) * 360.0f);
    }
}

void EphemerisModel::_calculateIllumFactors()
{
    // saving sun vector for future purposes
    _sunVec = _skyDome->getSunDir();
    osg::Vec4 position(_sunVec[0], _sunVec[1], _sunVec[2], 0.0f);

    // just save calculated illumination
    _illumination = _skyDome->getIllumination();
    //_uniformSunIntensity->set(_illumination);

    // setting the GL light
    osg::Light & light = *(_sunLightSource->getLight());

    // ambient-diffuse when fog - increase ambient, decrease diffuse
    osg::Vec3f cFogAmb = _skyDome->getAmbient();
    osg::Vec3f cFogDif = _skyDome->getDiffuse();
    // that value will be subtracted from diffuse
    const osg::Vec3f cFogDifCut = cFogDif * (0.5f * _fogDensity);
    cFogDif -= cFogDifCut;
    // and add it to ambient but with half weight
    cFogAmb += cFogDifCut * 0.5f;
    // load to GL
    light.setAmbient(osg::Vec4f(cFogAmb, 0.f));
    light.setDiffuse(osg::Vec4f(cFogDif, 0.f));

    // dim specular when fog is high
    osg::Vec3f cFogSpec = _skyDome->getSpecular() * (1.f - _fogDensity * _fogDensity);
    // load to GL
    light.setSpecular(osg::Vec4f(cFogSpec, 0.f));

    // Convert from calculated LTP position to current LSC position
    FIXME(Coords);
    // position = position * utils::GetCoordinateSystem()->GetLTP2LCSMatrix();
    light.setPosition(position);

    _lightDirUniform->set( position * _mv /*_ephem->getModelViewMatrix()*/ /*osg::Vec4(lightDir,1.)*/);
    
    FIXME("Что-то новое");
    const float fIllumDiffuseFactor = 1.f - _skyClouds->getOvercastCoef();
    //_illumination = cg::luminance_crt(cFogAmb + cFogDif * fIllumDiffuseFactor);

    FIXME (Нелепые цветные фантазии) 
    osg::Vec4 diffuse (cFogDif*1.2f, 1.0);   
    osg::Vec4 ambient (cFogAmb * 1.3 *(1 - 0.75 * _skyClouds->getOvercastCoef()), 1.0);      
    osg::Vec4 specular (cFogSpec * 1.2, 1.0);     
    _illumination = cg::luminance_crt(ambient + diffuse * fIllumDiffuseFactor);


    // when ambient is low - get it's color directly (to make more realistic fog at dusk/dawn)
    const float fFogDesatFactor = _skyDome->getAmbient()[0];
    _cFogColor = cg::lerp01( _skyDome->getAmbient(), osg::Vec3f(_illumination, _illumination, _illumination),fFogDesatFactor);
    // some tweaks to make it a little bit more "colder"
    _cFogColor.x() *= 0.945f;
    _cFogColor.y() *= 0.960f;

    // save fog
    _skyFogLayer->setFogParams(_cFogColor, _fogDensity);

    
    _uniformSunIntensity->set(_illumination);
    
    // notify starts and moon about characteristics
    _skyStarField->setIlluminationFog(_illumination, _fogDensity);
    osg::Vec3f cMoonColor = _skyDome->getSpecular();
    cMoonColor /= std::max(cMoonColor.x(), std::max(cMoonColor.y(), cMoonColor.z()));
    _skyMoon->setMoonReflColor(cMoonColor);

#if 0
    FIXME(Свет здесь);
    osg::Vec4 diffuse  = osg::Vec4(cFogDif/**1.6f*/,1.0f);     //sls->getDiffuse();
    osg::Vec4 ambient  = osg::Vec4(cFogAmb/**1.2*/,1.0f);      //sls->getAmbient();
    osg::Vec4 specular = osg::Vec4(cFogSpec/**1.6*/,1.0f);     // sls->getSpecular();
#endif

    ambient.w() = _illumination ;
    specular.w() = avCore::GetEnvironment()->GetWeatherParameters().RainDensity;                 

    _specularUniform->set(specular);
    _ambientUniform->set(ambient);
    _diffuseUniform->set(diffuse);
    
    avCore::GetEnvironment()->setIllumination(_illumination);

}


// this method will move in Clouds object in future
void EphemerisModel::SetClouds( int nType, float fCloudIntensity )
{
    // avAssertMessage(nType >= 0 && nType < 4, "Unknown clouds type");

    FIXME("Чет перебор с облаками, хотя некоторые ракурсы ничего так получаются");
    if (nType != _nCloudsType)
    {
        _nCloudsType = nType;

        char szBuffer[64];
        sprintf_s(szBuffer, sizeof(szBuffer), "Sky/Clouds%d.dds", /*_nCloudsType + 1*/2 );

        osg::Texture2D * pTexture = new osg::Texture2D(osgDB::readImageFile(szBuffer,new osgDB::Options));//utils::GetDatabase()->LoadTexture(szBuffer);
        _skyDome->setCloudsTexture(pTexture);

        _skyClouds->setCloudsTexture(static_cast<av::weather_params::cloud_type>(_nCloudsType));
        _skyClouds->setCloudsDensity(fCloudIntensity);
        _needsToUpdate = true;
    }
}

void EphemerisModel::_setDefault()
{
    setLatitudeLongitude(osg::DegreesToRadians(59.42), osg::DegreesToRadians(10.47)); // Horten
    setHumidityTemperature(0.1f, 10.0f);                                              // almost no haze, T = 10 deg celcius
    setDateTime(avSky::DateTime(2010, 6, 22, 13, 30, 0));                             // June 22, half-an-hour before lunch
    setManualMode(false, 1.0f);                                                       //
}

// fog density control
void EphemerisModel::setFogDensity(float fFogDensity)
{
    if (!cg::eq(fFogDensity, _fogDensity, 0.005f))
    {
        _fogDensity = fFogDensity;
        _needsToUpdate = true;
    }
}

// sun luminance manual mode control
void EphemerisModel::setManualMode(bool bManual, float fIllumDesired)
{
    // proceed sky-dome update if mode changed
    if (_manualSunElevationMode != bManual)
    {
        _manualSunElevationMode = bManual;
        _needsToUpdate = true;
    }

    // if manual mode - test if illumination changed
    if (bManual && !cg::eq(fIllumDesired, _desiredIllumination, 0.005f))
    {
        _desiredIllumination = fIllumDesired;
        _needsToUpdate = true;
    }
}

// force update underwater color
void EphemerisModel::updateUnderWaterColor( const osg::Vec3f & cWaterColor )
{
    FIXME(Water color);
    // _skyFogLayer->SetUnderWaterColor(cWaterColor * _illumination);
}

// called once before everything
bool EphemerisModel::PreUpdate()
{
    // Set position
    FIXME(Hardcode)
    auto cOrigin = ::get_base();
    cOrigin.lat = 43.44444;
    cOrigin.lon = 39.94694;
    setLatitudeLongitude(cOrigin.lat, cOrigin.lon);

    // Set humidity and temperature
    const avCore::Environment::EnvironmentParameters & cEnvironmentParameters = avCore::GetEnvironment()->GetEnvironmentParameters();
    const float airTemperatureInCelsius = cEnvironmentParameters.AirTemperature - 273.0f;
    setHumidityTemperature(cEnvironmentParameters.AirHumidity, airTemperatureInCelsius);

    // Set data and time
    const avCore::Environment::TimeParameters & cTime = avCore::GetEnvironment()->GetTimeParameters();
    setDateTime(avSky::DateTime(cTime.Year, cTime.Month, cTime.Day, cTime.Hour, cTime.Minute, cTime.Second));

    // Set manual illumination setup
    const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
    setManualMode(!cIlluminationParameters.IsAutoIllumination, cIlluminationParameters.Illumination);


    const avCore::Environment::WeatherParameters & cWeatherParameters = avCore::GetEnvironment()->GetWeatherParameters();

    // Set clouds
    SetClouds((int)cWeatherParameters.CloudType,cWeatherParameters.CloudDensity);

    // Update fog density (unit value - must somehow correspond to real physical values inside)
    // Fog color is computed afterwards in SkyDome, based on sun intensity and so on, saved also there
    // Global Hail, Rain and Snow also affects fog density, snow especially
    const float
        fPrecipitationFogImpact = 0.7f * powf(osg::maximum(cWeatherParameters.SnowDensity, osg::maximum(cWeatherParameters.RainDensity * 0.8f, cWeatherParameters.HailDensity * 0.5f)), 0.5f);
    setFogDensity(cg::lerp01( fPrecipitationFogImpact, 1.f, cWeatherParameters.FogDensity));

    // xEvguenZ: we must take proper care of clouds density, because current realization, which was here, looked not sufficient at all
    _fCloudsDensity = cWeatherParameters.CloudDensity;

    return true;
}
