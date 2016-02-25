#pragma once 

// FIXME env or weather or meteo
//////////////////////////////////////////
namespace meteo 
{

    struct local_params
    {
        float wind_speed, wind_azimuth, wind_gusts;
        // cg::point_2f wind_dir;

        local_params() 
            : wind_speed(0)
            , wind_azimuth(0)
            , wind_gusts(0)
        {}

        local_params(float wind_speed, float wind_azimuth, float wind_gusts) 
            : wind_speed  (wind_speed)
            , wind_azimuth(wind_azimuth)
            , wind_gusts  (wind_gusts)
        {}
    };

    REFL_STRUCT(local_params)
        REFL_ENTRY(wind_speed)
        REFL_ENTRY(wind_azimuth)
        REFL_ENTRY(wind_gusts)
        REFL_END()

}
//////////////////////////////////////////