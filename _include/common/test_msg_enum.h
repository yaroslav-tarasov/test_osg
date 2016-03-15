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
			id_destroy          ,
            id_ready            ,

            id_state            ,
            id_props_updated    ,
            id_vis_peers        ,


            am_malfunction      ,
            am_engines_state    ,

            vm_attach_tow       ,
            vm_detach_tow       ,
            vm_detach_tow_dc    ,
            vm_fire_fight       ,
			
			id_environment      ,   

            sm_container_msg    
       
        };
    }

}