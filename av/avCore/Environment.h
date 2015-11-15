#pragma once

namespace avCore
{

class Environment
{
public:
    
    typedef std::function<void(float)>  OnIlluminationChangeF;
    typedef std::function<void(float, float)> OnVisibleRangeChangeF;

    struct WeatherParameters
    {
        int   CloudType;
        float CloudDensity;

        float FogDensity;
        float RainDensity;
        float SnowDensity;
        float HailDensity;

        bool LocalFogBanksEnabled;
        bool LocalRainBanksEnabled;
        bool LocalSnowBanksEnabled;
        bool LocalHailBanksEnabled;

        WeatherParameters()
            : CloudType(0) // 2
            , CloudDensity(0.0f)
            , FogDensity(0.0f)
            , RainDensity(0.0f)
            , SnowDensity(0.0f)
            , HailDensity(0.0f)
            , LocalFogBanksEnabled(true)
            , LocalRainBanksEnabled(true)
            , LocalSnowBanksEnabled(true)
            , LocalHailBanksEnabled(true)
        {
        }
    };

    struct IlluminationParameters
    {
        bool IsAutoIllumination;
        float Illumination;
        float NavigationLightsThreshold;
        float CulturalLightsLightsThreshold;

        IlluminationParameters()
            : IsAutoIllumination(true)
            , Illumination(1.0f)
            , NavigationLightsThreshold(0.5f)
            , CulturalLightsLightsThreshold(0.5f)
        {
        }
    };

    struct EnvironmentParameters
    {
        float         AirTemperature;
        float         AirHumidity;
        cg::point_3f  WindDirection;
        float         WindSpeed;

        EnvironmentParameters()
            : AirTemperature(280.0f)
            , AirHumidity(0.0f)
            , WindDirection(0,0,0)
            , WindSpeed(0.1f)
        {
        }
    };

    struct TimeParameters
    {
        unsigned Year;
        unsigned Month;
        unsigned Day;
        unsigned Hour;
        unsigned Minute;
        unsigned Second;

        TimeParameters()
            : Year(2015)
            , Month(1)
            , Day(1)
            , Hour(12)
            , Minute(0)
            , Second(0)
        {
        }
    };


private:

    Environment();

public:

    ~Environment();

    static Environment * GetInstance();
    static void          Create();
    static void          Release();

    inline const WeatherParameters &      GetWeatherParameters()      const { return m_WeatherParameters;      }
    inline const IlluminationParameters & GetIlluminationParameters() const { return m_IlluminationParameters; }
    inline const EnvironmentParameters &  GetEnvironmentParameters()  const { return m_EnvironmentParameters;  }
    inline const TimeParameters &         GetTimeParameters()         const { return m_TimeParameters;         }
    
    void setIllumination (float Illumination)              { m_IlluminationParameters.Illumination = Illumination; if(ic_) ic_(Illumination); }
    void setVisibleRange (float VisibleRange,float ExpDensity)              { if(vrc_)  vrc_(VisibleRange,ExpDensity); }
    void setCallBacks    (const OnIlluminationChangeF& ic, const OnVisibleRangeChangeF& vrc) {ic_ = ic; vrc_=vrc; } 
public:

    WeatherParameters      m_WeatherParameters;
    EnvironmentParameters  m_EnvironmentParameters;
    TimeParameters         m_TimeParameters;

private:
    IlluminationParameters m_IlluminationParameters;
    OnIlluminationChangeF  ic_;
    OnVisibleRangeChangeF  vrc_;

    static Environment * m_pInstance;
};

__forceinline Environment * GetEnvironment() {return Environment::GetInstance();}

}
