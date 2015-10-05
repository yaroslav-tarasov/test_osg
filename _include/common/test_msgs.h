#pragma once 

#include "network/msg_base.h"
using network::gen_msg;

namespace net_layer
{

    namespace test_msg
    {
        enum message_id
        {
            id_setup            ,
            id_run              ,
            id_create           ,
            id_ready            ,
        };


        struct setup
            : network::msg_id<id_setup>
        {
            setup(double task_time = 0., double srv_time = 0.,std::string icao_code = "URSS")
                : task_time (task_time)
                , srv_time  (srv_time )
                , icao_code (icao_code)
            {
            }

            double      task_time;
            double      srv_time;
            std::string icao_code;
        };
        
        REFL_STRUCT(setup)
            REFL_ENTRY(task_time)
            REFL_ENTRY(srv_time )
            REFL_ENTRY(icao_code )
        REFL_END()

        struct run
            : network::msg_id<id_run>
        {
            run(double task_time = 0., double srv_time = 0.)
                : task_time (task_time)
                , srv_time  (srv_time )
            {
            }

            double  task_time;
            double  srv_time;
        };

        REFL_STRUCT(run)
            REFL_ENTRY(task_time)
            REFL_ENTRY(srv_time )
        REFL_END()

        struct create
            : network::msg_id<id_create>
        {
            create(double lat = 0., double lon = 0., double course = 0.)
                : lat    (lat)
                , lon    (lon )
                , course (course)
            {
            }

            double  lat;
            double  lon;
            double  course;
        };

        REFL_STRUCT(create)
            REFL_ENTRY(lat)
            REFL_ENTRY(lon )
            REFL_ENTRY(course )
        REFL_END()

        typedef gen_msg<id_ready, uint16_t> ready_msg;
    }


}