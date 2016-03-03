#include "stdafx.h"

#include "av/precompiled.h"

#include "av/avCore/Utils.h"
#include "av/avScene/Scene.h"
#include "av/avWeather/Weather.h"

using namespace avCore;


Environment * Environment::m_pInstance = NULL;


//////////////////////////////////////////////////////////////////////////
Environment::Environment()
{
}

//////////////////////////////////////////////////////////////////////////
Environment::~Environment()
{
}

//////////////////////////////////////////////////////////////////////////
Environment * Environment::GetInstance()
{
    avAssert(m_pInstance);
    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////
void Environment::Create()
{
    avAssert(m_pInstance == NULL);
    m_pInstance = new Environment();
}

//////////////////////////////////////////////////////////////////////////
void Environment::Release()
{
    avAssert(m_pInstance != NULL);
    delete m_pInstance;
    m_pInstance = nullptr;
}   

void Environment::SetEnvironment( environment_params const & p )
{
#if 0
    cg::geo_point_2 geo_pos;
    time_t          utc_time;
    float           night_threshold;
#endif

    m_EnvironmentParameters.AirHumidity    = p.air_temperature;
    m_EnvironmentParameters.AirTemperature = p.air_humidity;

}

environment_params const & Environment::GetEnvironment() const
{
    static environment_params ep;
    ep.air_humidity    = m_EnvironmentParameters.AirHumidity;
    ep.air_temperature = m_EnvironmentParameters.AirTemperature;
    return ep;
}

// global weather params
void Environment::SetWeather( weather_params const & wp )
{
    m_EnvironmentParameters.WindDirection  = cg::point_3f(wp.wind_dir);
    m_EnvironmentParameters.WindSpeed      = wp.wind_speed;
    m_WeatherParameters.FogDensity         = wp.fog_density;
    m_WeatherParameters.RainDensity        = wp.rain_density;
    m_WeatherParameters.SnowDensity        = wp.snow_density;
    m_WeatherParameters.HailDensity        = wp.hail_density;
    m_WeatherParameters.CloudDensity       = wp.clouds_density;
    m_WeatherParameters.CloudType          = wp.clouds_type;
    m_WeatherParameters.LightningIntencity = wp.lightning_intensity;
}

weather_params const & Environment::GetWeather() const
{
     static weather_params wp;
     wp.wind_speed      = m_EnvironmentParameters.WindSpeed;
     wp.wind_gusts      = 0;
     wp.wind_dir        = m_EnvironmentParameters.WindDirection;
     wp.fog_density     = m_WeatherParameters.FogDensity;
     wp.rain_density    = m_WeatherParameters.RainDensity;
     wp.snow_density    = m_WeatherParameters.SnowDensity;
     wp.hail_density    = m_WeatherParameters.HailDensity;
     wp.clouds_density  = m_WeatherParameters.CloudDensity;

     wp.clouds_type     = 
         static_cast<av::weather_params::cloud_type>(m_WeatherParameters.CloudType);
     
     wp.lightning_intensity  = m_WeatherParameters.LightningIntencity;
     
     return wp;
}

void Environment::UpdateLocalBank( local_bank_id_t id, local_bank_data const & data )
{
    if (avScene::GetScene() && avScene::GetScene()->getWeather())
    {
        avWeather::Weather * pWeatherNode = avScene::GetScene()->getWeather();

        const avWeather::Weather::WeatherBankIdentifier nID = id;
        const double dLatitude      = data.pos.x;
        const double dLongitude     = data.pos.y;
        const float fHeading        = data.heading;
        const float fEllipseRadX    = data.ellipse.x;
        const float fEllipseRadY    = data.ellipse.y;
        FIXME(Высота?)
        const float fHeight         = 1000.f;
        const avWeather::PrecipitationType ptType = avWeather::PrecipitationType(data.type);
        const float fIntensity      = data.intensity;
        const float fCentralPortion = data.central_portion;

        pWeatherNode->UpdateLocalWeatherBank(nID, 
            dLatitude, dLongitude, fHeading, 
            fEllipseRadX, fEllipseRadY, fHeight, 
            ptType, fIntensity, fCentralPortion);
    }
}

void Environment::RemoveLocalBank( local_bank_id_t id ) 
{
    if (avScene::GetScene() && avScene::GetScene()->getWeather())
    {
        avWeather::Weather * pWeatherNode = avScene::GetScene()->getWeather();

        const avWeather::Weather::WeatherBankIdentifier nID = id;
        pWeatherNode->RemoveLocalWeatherBank(nID);
    }
}

