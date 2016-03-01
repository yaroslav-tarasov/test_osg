#pragma once

namespace av
{

    // environment params (position, time, day/night, etc)
    struct environment_params
    {
        float air_temperature;
        float air_humidity;

        cg::geo_point_2 geo_pos;
        time_t utc_time;

        float night_threshold;

        environment_params() : air_temperature(+20.0f), air_humidity(0.3f), geo_pos(30., 30.), utc_time(20000), night_threshold(0.3f) {}
    };

    // weather params (wind, fog, haze, clouds, etc)
    struct weather_params
    {
        float wind_speed, wind_gusts;
        cg::point_2f wind_dir;

        enum precipitation_type
        {
            fog,
            rain,
            snow,
            hail,
            cloud,
            precipitations_types_num
        };

        float fog_density;
        float rain_density;
        float snow_density;
        float hail_density;

        enum cloud_type
        {
            none,
            cloudy,
            cirrus,
            overcast,
            clouds_types_num
        } clouds_type;
        float clouds_density;

        weather_params() : wind_speed(0), wind_gusts(0),
            fog_density(0), rain_density(0), snow_density(0), hail_density(0),
            clouds_type(cirrus), clouds_density(1) {}
    };

    // illumination data to use in external logics on CPU side
    struct atmospehere_illumination_data
    {
        cg::point_3f scene_light_dir;
        float illumination;
        cg::colorf ambient, diffuse, specular;
        bool night_mode;
        cg::colorf fog_color;
        float fog_exp_coef;

        atmospehere_illumination_data() : scene_light_dir(0, 0, 1), illumination(1), night_mode(false), fog_exp_coef(0) {}
    };
}
