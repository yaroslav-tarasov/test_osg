#pragma once

#include "reflection/proc/binary.h"
#include "objects/environment_fwd .h"

namespace environment
{
    struct weather_t
    {
        weather_t()
            : air_temperature(20.f)
            , air_humidity(0.5f)

            , wind_speed(0.f)
            , wind_gusts(0.f)

            , fog_density(0.1f)
            , rain_density(0.f)
            , snow_density(0.f)
            , hail_density(0.f)

            , clouds_type(2)
            , clouds_density(1.f)
        {}

        float air_temperature;
        float air_humidity;

        float wind_speed;
        float wind_gusts;
        cg::point_2f wind_dir;

        float fog_density;
        float rain_density;
        float snow_density;
        float hail_density;

        unsigned clouds_type;
        float clouds_density;
    };

    struct lighting_t
    {
        lighting_t()
            : night_threshold(0.5)
        {}

        float night_threshold ;
    };

    struct settings_t
    {
        settings_t()
            : demo_weather(false)
            , time_factor(1.)
            , start_time(0.)
        {}

        bool demo_weather;
        double time_factor;

        weather_t  weather ;
        lighting_t lighting ;

        double start_time ;
    };

REFL_STRUCT(weather_t)
    REFL_NUM(air_temperature, -50.f, 50.f, 0.1f)
    REFL_NUM(air_humidity, 0.f, 1.f, 0.01f)

    REFL_NUM(wind_speed, 0.f, 200.f, 0.1f)
    REFL_NUM(wind_gusts, 0.f, 200.f, 0.1f)
    REFL_ENTRY (wind_dir)

    REFL_NUM(fog_density, 0.f, 1.f, 0.01f)
    REFL_NUM(rain_density, 0.f, 1.f, 0.01f)
    REFL_NUM(snow_density, 0.f, 1.f, 0.01f)
    REFL_NUM(hail_density, 0.f, 1.f, 0.01f)

    REFL_ENUM(clouds_type, ("None", "Cloudy", "Cirrus", "Overcast", NULL))
    REFL_NUM(clouds_density, 0.f, 1.f, 0.01f)
REFL_END()

REFL_STRUCT(lighting_t)
    REFL_NUM(night_threshold, 0.f, 1.f, 0.01f)
REFL_END()

REFL_STRUCT(settings_t)
    REFL_NUM(time_factor, 0.1, 10000., 0.1)    
    REFL_ENTRY(demo_weather)
    REFL_ENTRY(weather)
    REFL_ENTRY(lighting)
    REFL_SER(start_time)
REFL_END()

} // namespace environment
