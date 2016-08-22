#include "stdafx.h"
#include "precompiled_objects.h"

#include "environment_visual.h"

#include "common/airport.h"

namespace environment
{

    object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
    {
        return object_info_ptr(new visual(oc, dict));
    }

    AUTO_REG_NAME(environment_visual, visual::create);

    inline double demo_fog_density(int preset)
    {
        switch (preset)
        {
        case 0:
            return 0.1;
        case 1:
            return 0.65;
        case 2:
            return 0.4;
        case 3:
            return 0.0;
        }

        return 0;
    }

    inline double demo_rain(int preset)
    {
        switch (preset)
        {
        case 0:
            return 0.;
        case 1:
            return 0.;
        case 2:
            return 1.;
        case 3:
            return 0.;
        }

        return 0;
    }

    inline av::weather_params::cloud_type demo_clouds(int preset)
    {
        switch (preset)
        {
        case 0:
            return av::weather_params::cirrus;
        case 1:
            return av::weather_params::cloudy;
        case 2:
            return av::weather_params::overcast;
        case 3:
            return av::weather_params::none;
        }

        return av::weather_params::none;
    }

    visual::visual( kernel::object_create_t const& oc, dict_copt dict )
        : view(oc, dict)
        , sys_(dynamic_cast<visual_system *>(oc.sys))
        , current_time_(settings_.start_time)
        , is_visible_(false)
        , ani_       (kernel::find_first_object<ani_object::info_ptr>(collection_))
    {
        on_settings_changed();
    }

    void visual::on_object_destroying(kernel::object_info_ptr obj)
    {
        if (ani_ == obj)
        {
            ani_.reset();
            airport_.reset();
        }
    }

    void visual::update( double time )
    {
        if (last_update_time_)
            current_time_ += settings_.time_factor * (time - *last_update_time_) ;

        if (is_visible_)
        {
            time_t utc_time = current_time_;

            av::environment_params env ;

            env.air_temperature = settings_.weather.air_temperature ;
            env.air_humidity    = settings_.weather.air_humidity ;
            env.utc_time        = utc_time;
            env.geo_pos         = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point; // TODO

            FIXME ("Погода и время, utc_time useless")
            //sys_->scene()->get_env_weather()->set_environment(env);
            sys_->scene()->getEnvWeather()->SetEnvironment(env) ;

            if (settings_.demo_weather)
            {  
                using namespace boost::posix_time;

                ptime st = from_time_t(settings_.start_time);
                ptime t = from_time_t(utc_time);
                uint32_t days = t.date().day_of_year() - st.date().day_of_year();

                int preset = days % 4;
                //int prev_preset = preset == 0 ? 3 : preset - 1;
                int next_preset = (days+1) % 4;

                double fog_density       = demo_fog_density(preset);
                double next_fog_density  = demo_fog_density(next_preset);
                double rain_density      = demo_rain(preset);
                double next_rain_density = demo_rain(next_preset);
                auto clouds = demo_clouds(preset);
                auto next_clouds = demo_clouds(next_preset);

                time_duration dur = t.time_of_day();
                double fog = cg::clamp(21 * 60. * 60., 24 * 60. * 60., fog_density, next_fog_density)(dur.total_seconds());
                double rain = cg::clamp(21 * 60. * 60., 24 * 60. * 60., rain_density, next_rain_density)(dur.total_seconds());

                av::weather_params wth ;
                wth.fog_density    = fog ;
                wth.wind_speed     = settings_.weather.wind_speed ;
                wth.wind_dir       = settings_.weather.wind_dir ;
                wth.wind_gusts     = settings_.weather.wind_gusts ;
                wth.rain_density   = rain ;
                wth.snow_density   = settings_.weather.snow_density ;
                wth.hail_density   = settings_.weather.hail_density ;
                wth.clouds_type    = dur.hours() >= 20 ? next_clouds : clouds ;
                wth.clouds_density = settings_.weather.clouds_density ;
                
                // sys_->scene()->get_env_weather()->set_weather(wth) ;
                sys_->scene()->getEnvWeather()->SetEnvironment(env) ;
                sys_->scene()->getEnvWeather()->SetWeather(wth) ;
            }
        }

        last_update_time_ = time ;
    }

    void visual::on_parent_changed()
    {
        if (airport::vis_info_ptr vinfo = parent().lock())
        {
            is_visible_ = vinfo->is_visible() ;

            on_settings_changed() ;

            airport::info_ptr ainfo = vinfo;
            Assert(ainfo);
#if 0
            airport_ = ani_->navigation_info()->get_airport(ainfo->name());
#endif
        }
    }

    void visual::on_settings_changed()
    {
        if (!is_visible_)
            return ;

        av::environment_params env ;

        env.air_temperature = settings_.weather.air_temperature ;
        env.air_humidity    = settings_.weather.air_humidity ;
        env.geo_pos         = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point ; // TODO
        env.utc_time        = current_time_ ;

        av::weather_params wth ;

        wth.fog_density    = settings_.weather.fog_density ;
        wth.wind_speed     = settings_.weather.wind_speed ;
        wth.wind_dir       = settings_.weather.wind_dir ;
        wth.wind_gusts     = settings_.weather.wind_gusts ;
        wth.rain_density   = settings_.weather.rain_density ;
        wth.snow_density   = settings_.weather.snow_density ;
        wth.hail_density   = settings_.weather.hail_density ;
        wth.clouds_type    = (av::weather_params::cloud_type)settings_.weather.clouds_type ;
        wth.clouds_density = settings_.weather.clouds_density ;
        wth.lightning_intensity = settings_.weather.lightning_intensity ;

        //sys_->scene()->get_env_weather()->set_environment(env);
        sys_->scene()->getEnvWeather()->SetEnvironment(env) ;
        sys_->scene()->getEnvWeather()->SetWeather(wth) ;
    }   

    void visual::on_start_time_changed()
    {
        current_time_ = settings_.start_time ;
    }
}


