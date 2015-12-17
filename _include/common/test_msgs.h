#pragma once 


#include "network/msg_base.h"
using network::gen_msg;

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


// FIXME messages and namespaces

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
            vm_attach_tow       ,
            vm_detach_tow       ,
            vm_detach_tow_dc    ,
            sm_container_msg
        };


        struct setup
            : network::msg_id<id_setup>
        {
            setup(std::string icao_code = "URSS")
                : icao_code (icao_code)
            {
            }

            std::string icao_code;
        };
        
        REFL_STRUCT(setup)
            REFL_ENTRY(icao_code )
        REFL_END()

        struct state
            : network::msg_id<id_state>
        {
            state(double task_time, double srv_time, double factor)
                : task_time (task_time)
                , srv_time  (srv_time )
				, factor    (factor   )
            {
            }

            state()
                : task_time (0)
                , srv_time  (0)
				, factor    (0)
            {
            }

            double      task_time;
            double      srv_time;
			double      factor;
        };

        REFL_STRUCT(state)
            REFL_ENTRY(task_time)
            REFL_ENTRY(srv_time )
            REFL_ENTRY(factor )
        REFL_END()

        struct run
            : network::msg_id<id_run>
        {
            run()
                : ext_id(0)
                , keypoint (cg::point_3())
                , orien    (cg::quaternion()) 
                , time     (0)
                , speed    (0)
			    , reverse  (false)
				, mlp      (meteo::local_params())
            {
            }

            run( const uint32_t id, const cg::point_3&       keypoint, const cg::quaternion& orien, double speed, double time,  bool reverse, const meteo::local_params& mlp)
                : ext_id(id)
                , keypoint (keypoint)
                , orien (orien) 
                , time (time)
                , speed(speed)
				, reverse (reverse)
				, mlp  (mlp)
            {
            }

            uint32_t            ext_id;
            cg::point_3         keypoint;
            cg::quaternion      orien;
			double              time;
            double              speed;
			bool			    reverse;
			meteo::local_params mlp;
        };

        REFL_STRUCT(run)
            REFL_ENTRY( ext_id   )
            REFL_ENTRY( time     )
            REFL_ENTRY( speed    )
            REFL_ENTRY( keypoint )
            REFL_ENTRY( orien    )
			REFL_ENTRY( reverse  )
            REFL_ENTRY( mlp      )
        REFL_END()
        
        enum object_kind_t : int16_t
        {
            ok_none           ,
            ok_aircraft = 0x10,
            ok_helicopter     ,
            ok_aerostat       ,
            ok_vehicle = 0x100,
            ok_tow_tractor    ,
            ok_fire_engine    ,
            ok_bus            ,
            ok_cleaner        ,
            ok_viewer         ,
            ok_animal  = 0x200,
            ok_flock_of_birds ,
            // ok_size
        };

        struct create
            : network::msg_id<id_create>
        {
            
            create()
                : ext_id (0)
                , pos (pos)
                , orien (orien) 
            {
            }

            create(uint32_t id, const cg::point_3&       pos, const cg::quaternion& orien , object_kind_t ok, const std::string& model_name)
                : ext_id      (id)
                , pos (pos)
                , orien (orien)
                , object_kind (ok)
				, model_name (model_name)
            {
            }

            uint32_t            ext_id;
            cg::point_3         pos;
            cg::quaternion      orien;
            object_kind_t       object_kind;
			std::string         model_name;
            std::string         custom_label;
        };

        REFL_STRUCT(create)
            REFL_ENTRY( ext_id   )
            REFL_ENTRY( pos      )
            REFL_ENTRY( orien    )
			REFL_ENTRY( object_kind )
            REFL_ENTRY( model_name  )
            REFL_ENTRY( custom_label  )
        REFL_END()

        typedef gen_msg<id_ready, uint16_t> ready_msg;


// Типы и значения похожи/повторяют  внутренние для самолета
//  , но они принципиально другие 

    enum malfunction_kind_t
    {
        MF_CHASSIS_FRONT = 0,
        MF_CHASSIS_REAR_LEFT,
        MF_CHASSIS_REAR_RIGHT,
        MF_FIRE_ON_BOARD,
        MF_SMOKE_ON_BOARD,
        MF_FUEL_LEAKING,
        MF_ONE_ENGINE,
        MF_ALL_ENGINES,
    
        MF_SIZE
    };


    struct malfunction_msg
        : network::msg_id<am_malfunction>
    {
        malfunction_msg() {}

        malfunction_msg(uint32_t id, malfunction_kind_t kind, bool enabled)
            : ext_id      (id),kind(kind), enabled(enabled)
        {}
         
        uint32_t           ext_id;
        malfunction_kind_t kind;
        bool               enabled;
    };

    REFL_STRUCT(malfunction_msg)
        REFL_ENTRY(kind)
        REFL_ENTRY(enabled)
    REFL_END()

    typedef gen_msg<vm_attach_tow, uint32_t>               attach_tow_msg_t;    
    typedef gen_msg<vm_detach_tow, uint32_t>               detach_tow_msg_t;
    
    struct detach_tow_coords_msg_t
        : network::msg_id<vm_detach_tow_dc>
    {
        detach_tow_coords_msg_t() {}

        detach_tow_coords_msg_t(uint32_t id, cg::point_3f const& pos)
            : ext_id      (id), pos(pos)
        {}

        uint32_t           ext_id;
        cg::point_3f          pos;
    };
    
    REFL_STRUCT(detach_tow_coords_msg_t)
        REFL_ENTRY(ext_id)
        REFL_ENTRY(pos)
    REFL_END()

    struct container_msg
        : network::msg_id<sm_container_msg>
    {
        typedef std::vector<bytes_t>  msgs_t;

        container_msg(){}

        container_msg(msgs_t&& msgs)
            : msgs(move(msgs))
        {
        }

        msgs_t msgs;
    };

    REFL_STRUCT(container_msg)
        REFL_ENTRY(msgs)
    REFL_END()



    }





}