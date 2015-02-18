#pragma once

#include "phys/rigid_body_info.h"
#include "cpp_utils/polymorph_ptr.h"
#include "phys_sys.h"
#include "phys/phys_sys_common.h"
#include "vehicle.h"
#include "sensor.h"

namespace phys
{
namespace ray_cast_vehicle
{
    struct impl 
        : rigid_body_impl
        , btActionInterface
        , info
        , control
    {
        impl(system_impl_ptr sys, double mass, compound_sensor_ptr s,/*sensor_ptr s,*/ decart_position const& pos);
        ~impl();

        // rigid_body_impl
    private:
        bt_rigid_body_ptr get_body() const;
        void pre_update(double dt);
        void has_contact(rigid_body_user_info_t const* /*other*/, cg::point_3 const& /*local_point*/, cg::point_3 const& /*vel*/) {}

        // btActionInterface
    private:
        void updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep);
        void debugDraw(btIDebugDraw* debugDrawer);

    //info
    private:
        void add_wheel( double mass, double width, double radius, cg::point_3 const& offset, cg::cpr const & orien, bool has_damper );

        void set_steer   (double steer);
        void set_brake   (double brake);
        void set_thrust  (double thrust);

        decart_position get_position() const;
        decart_position get_wheel_position( size_t i ) const ;

        double get_tow_rod_course() const;

    //control
    private:
        void set_tow(rigid_body_ptr rb, cg::point_3 const& self_offset, cg::point_3 const& offset);
        void reset_tow();
        void set_course_hard(double course);
        void reset_suspension();

    private:
        system_impl_ptr                      sys_;

        bt_collision_shape_ptr               chassis_shape_;
        rigid_body_proxy                     chassis_;

        raycast_vehicle_proxy                raycast_veh_;

        double                               steer_;

        btRaycastVehicle::btVehicleTuning    tuning_;

        bt_rigid_body_ptr      tow_;
        bt_collision_shape_ptr tow_rod_shape_;
        
        rigid_body_proxy                            tow_rod_;
        constraint_proxy<bt_generic_constraint_ptr> tow_constraint_self_;
        constraint_proxy<bt_generic_constraint_ptr> tow_constraint_tow_;
    };
}
}
