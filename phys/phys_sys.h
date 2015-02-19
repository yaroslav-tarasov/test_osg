#pragma once

#include "phys/phys_sys_fwd.h"
//#include "sensor.h"
#include "objects/aircraft.h"
#include "position.h"


namespace phys
{
    
    struct compound_sensor_t
    {
        virtual cg::point_3 get_offset()  = 0;
        virtual ~compound_sensor_t() {}
    };

	struct sensor_t
	{
		virtual ~sensor_t() {}
	};

    struct rigid_body
    {
        virtual ~rigid_body() {}
    };

    struct static_mesh
    {
        virtual ~static_mesh() {}
    };

    struct static_convex
    {
        virtual ~static_convex() {}
    };

    struct system
    {
        virtual ~system() {}

        virtual void update( double dt ) = 0;
        //virtual void set_debug_renderer(victory::debug_render_ptr debug_render = victory::debug_render_ptr()) = 0;

        //virtual static_mesh_ptr            create_static_mesh       ( sensor_ptr s ) = 0;
        //virtual static_convex_ptr          create_static_convex     ( sensor_ptr s, point_3 const& pos, quaternion const& orien ) = 0;
        virtual ray_cast_vehicle::info_ptr create_ray_cast_vehicle  ( double mass, /*sensor_ptr s*/compound_sensor_ptr s, decart_position const& pos ) = 0;
        virtual aircraft::info_ptr         create_aircraft          ( aircraft::params_t const& params, compound_sensor_ptr s, decart_position const& pos ) = 0;
    };

    //! интерфейс коллизии, 
    struct collision
    {
        virtual ~collision() {}

        virtual optional<double> intersect_first(point_3 const& p, point_3 const& q) const = 0;
    };

    /*PHYS_SYS_API*/ system_ptr create_phys_system();

    // FIXME не здесь пожалуй не место

    namespace aircraft
    {
        compound_sensor_ptr fill_cs(nm::manager_ptr manager);
    }

    namespace ray_cast_vehicle
    {
        compound_sensor_ptr fill_cs(nm::manager_ptr manager);
    }

}


namespace phys
{
    struct control
    {
        virtual ~control() {}

        virtual system_ptr        get_system(size_t zone) = 0;

        virtual optional<size_t>  get_zone(cg::geo_point_3 const & pos) const = 0;
        virtual optional<size_t>  get_zone(std::string const& airport) const = 0;
        virtual cg::geo_base_3 const& get_base(size_t zone) const = 0;
        virtual std::string zone_name(size_t id) const = 0;

        //DECLARE_EVENT(zone_created, (size_t));
        //DECLARE_EVENT(zone_destroyed, (size_t));
    };



    typedef polymorph_ptr<control> control_ptr;
}