#pragma once

#include <btBulletDynamicsCommon.h> 
#include "rigid_body_info.h"
#include "utils/polymorph_ptr.h"

namespace phys
{
    typedef osg::Vec3d decart_position;
    typedef osg::Vec3d point_3;
    typedef osg::Quat  cpr;

    struct contact_info_t
    {
        contact_info_t() {}
        contact_info_t( point_3 const& offset, point_3 const& vel )
            : vel(vel), offset(offset)
        {}

        point_3 vel;
        point_3 offset;
    };

    struct params_t
    {
        params_t()
            : min_aerodynamic_speed(50)
            , roll_sliding(0.1)
            , roll_omega_y(1.)
            , roll_omega_z(1.)

            , course_sliding(0.1)
            , course_omega_z(1.)
            , course_omega_y(1.)

            , pitch_drag(1.)
            , pitch_attack(0.1)
            , pitch_omega_x(1.)
            , pitch_attack_derivative(1.)
            , elevator(200)
            , rudder(200)
            , ailerons(200)
            , aa0(-1.)
        {}

        double S;
        double chord;
        double length;
        double mass;
        double wingspan;

        double min_aerodynamic_speed;

        double Cl;
        double Cd0;
        double Cd2;
        double ClAOA;
        double Cs;
        double aa0;

        double thrust;
        double elevator;
        double rudder;
        double ailerons;

        double roll_sliding;
        double roll_omega_y;
        double roll_omega_z;

        double course_sliding;
        double course_omega_z;
        double course_omega_y;

        double pitch_drag;
        double pitch_attack;
        double pitch_omega_x;
        double pitch_attack_derivative;
    };   

    struct info
    {
        virtual ~info() {}

        virtual decart_position get_position() const = 0;
        virtual decart_position get_wheel_position( size_t i ) const = 0;
        virtual double Ixx() const = 0;
        virtual double Iyy() const = 0;
        virtual double Izz() const = 0;
        virtual params_t const& params() const = 0;
        virtual double drag() const = 0;
        virtual double lift() const = 0;
        virtual double thrust() const = 0;
        virtual bool has_contact() const = 0;
        virtual std::vector<contact_info_t> get_body_contacts() const = 0;
        virtual bool has_wheel_contact(size_t id) const = 0;
        virtual double wheel_skid_info(size_t id) const = 0;
    };  

    struct control : info
    {
        virtual size_t add_wheel( double mass, double width, double radius, point_3 const& offset, cpr const & orien, bool has_damper, bool is_front ) = 0;
        virtual void remove_wheel(size_t id) = 0;

        virtual void set_control_manager(std::function<void(double)> f) = 0;

        virtual void set_steer   (double steer) = 0;
        virtual void set_brake   (double brake) = 0;
        virtual void set_thrust  (double thrust) = 0;
        virtual void set_elevator(double elevator) = 0;
        virtual void set_ailerons(double ailerons) = 0;
        virtual void set_rudder  (double rudder) = 0;
        virtual void set_wind    (point_3 const& wind) = 0;
        virtual void apply_force (point_3 const& f) = 0;
        virtual void update_aerodynamics(double dt) = 0;
        virtual void reset_suspension() = 0;
    };

   class aircraft 
       : public rigid_body_user_info_t
       , public btActionInterface
   {
   public:
      aircraft(/*compound_sensor_t const* s,*/ params_t const& params, decart_position const& pos);

   private:
       void updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep);
       void debugDraw(btIDebugDraw* debugDrawer);

   private:
       //compound_shape_proxy                 chassis_shape_;
       //rigid_body_proxy                     chassis_;
       //raycast_vehicle_proxy                raycast_veh_;
       polymorph_ptr<rigid_body_user_info_t>  self_;
       btRaycastVehicle::btVehicleTuning    tuning_;

       std::function<void(double)> control_manager_;

       params_t params_;

       double   steer_;
       double   elevator_;
       double   ailerons_;
       double   rudder_;
       double   thrust_;

       point_3  wind_;

       double   prev_attack_angle_;

       bool     has_chassis_contact_;

       //typedef cg::duplicate_points_fixed_id<point_3> dup_points_t;
       //dup_points_t  body_contact_points_;

       struct contact_t
       {
           contact_t() {}
           contact_t(point_3 const& vel)
               : sum_vel(vel)
               , count(1)
           {}

           point_3 sum_vel;
           size_t count;
       };

       //fixed_id_vector<contact_t> body_contacts_;
       
       //fixed_id_vector<size_t> wheels_ids_;
   };
}