#pragma once 

#include "network/msg_base.h"

namespace net_layer
{

    namespace test_msg
    {
        enum message_id
        {
            id_setup            ,
            id_run              ,
        };


        struct setup
            : network::msg_id<id_setup>
        {
            setup(double task_time = 0., double srv_time = 0.)
                : task_time (task_time)
                , srv_time  (srv_time )
            {
            }

            double  task_time;
            double  srv_time;
        };
        
        REFL_STRUCT(setup)
            REFL_ENTRY(task_time)
            REFL_ENTRY(srv_time )
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

    }


}