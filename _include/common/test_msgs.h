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
            
            am_malfunction      ,
            sm_container_msg
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
            run()
                : ext_id(0)
                , keypoint (cg::point_3())
                , orien    (cg::quaternion()) 
                , time     (0)
                , speed    (0)
				, mlp      (meteo::local_params())
            {
            }

            run( const uint32_t id, const cg::point_3&       keypoint, const cg::quaternion& orien, double speed, double time, const meteo::local_params& mlp)
                : ext_id(id)
                , keypoint (keypoint)
                , orien (orien) 
                , time (time)
                , speed(speed)
				, mlp  (mlp)
            {
            }

            uint32_t            ext_id;
            cg::point_3         keypoint;
            cg::quaternion      orien;
            double              speed;
            double              time;
			meteo::local_params mlp;
        };

        REFL_STRUCT(run)
            REFL_ENTRY( ext_id   )
            REFL_ENTRY( time     )
            REFL_ENTRY( speed    )
            REFL_ENTRY( keypoint )
            REFL_ENTRY( orien    )
            REFL_ENTRY( mlp      )
        REFL_END()

        struct create
            : network::msg_id<id_create>
        {
            
            create()
                : ext_id (0)
                , lat    (0)
                , lon    (0 )
                , course (0)
            {
            }

            create(uint32_t id, double lat, double lon, double course , const std::string& object_type)
                : ext_id      (id)
                , lat         (lat)
                , lon         (lon )
                , course      (course)
				, object_type (object_type)
            {
            }

            uint32_t    ext_id;
            double      lat;
            double      lon;
            double      course;
			std::string object_type;
        };

        REFL_STRUCT(create)
            REFL_ENTRY(ext_id)
            REFL_ENTRY(lat)
            REFL_ENTRY(lon )
            REFL_ENTRY(course )
			REFL_ENTRY(object_type )
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
         
        uint32_t    ext_id;
        malfunction_kind_t kind;
        bool enabled;
    };

    REFL_STRUCT(malfunction_msg)
        REFL_ENTRY(kind)
        REFL_ENTRY(enabled)
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