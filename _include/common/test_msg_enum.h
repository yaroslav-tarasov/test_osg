#pragma once 

namespace net_layer
{

    namespace msg
    {
        enum message_id
        {
            id_setup            ,
            id_run              ,
            id_create           ,
            id_ready            ,

            id_state            ,

            am_malfunction      ,
            am_engines_state    ,

            vm_attach_tow       ,
            vm_detach_tow       ,
            vm_detach_tow_dc    ,
            vm_fire_fight       ,

            sm_container_msg    ,
            id_vis_peers        
        };
    }

}