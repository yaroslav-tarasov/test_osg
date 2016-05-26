#pragma once 

namespace net_layer
{

    namespace msg
    {
        enum message_id
        {
            id_setup            ,
            id_ready            ,

            id_state            ,
            id_props_updated    ,
            id_vis_peers        ,
            
            id_run              ,
            id_create           ,
            id_destroy          ,

            id_traj_assign      ,

            am_malfunction      ,
            am_engines_state    ,

            vm_attach_tow       ,
            vm_detach_tow       ,
            vm_detach_tow_dc    ,
            vm_fire_fight       ,
			
            ar_set_target       ,

			id_environment      ,   

            sm_container_msg    
       
        };
    }

}