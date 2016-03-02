#pragma once

namespace environment
{

    struct control
    {
         virtual ~control(){};
         virtual void set_weather(const weather_t&) = 0;
    };

}



