#pragma once

namespace vehicle
{

    struct control
    {
         virtual ~control(){};
         virtual void goto_pos(geo_point_2 pos,double course)=0;
    };

}



