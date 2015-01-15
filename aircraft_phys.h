#pragma once

#include "bi/rigid_body_info.h"
#include "cpp_utils/polymorph_ptr.h"
#include "bi/phys_sys_common.h"
#include "aircraft.h"

namespace phys
{
	namespace aircraft
	{

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
            , S(0.0),chord(0.0),length(0.0),mass(0.0),wingspan(0.0)
            , Cl(0.0)
            , Cd0(0.0)
            , Cd2(0.0)
            , ClAOA(0.0)
            , Cs(0.0)
            , thrust(0.0)

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


   
   ATTRIBUTE_ALIGNED16 (class impl) 
       : public rigid_body_user_info_t
	   , public rigid_body_impl
       , public btActionInterface
	   , public control
   {
   public:
      impl(system_impl_ptr,/*compound_sensor_t const* s,*/compound_shape_proxy& s, params_t const& params, decart_position const& pos);
   
   
   public:
	   size_t add_wheel( double mass, double width, double radius, point_3 const& offset, cpr const & orien, bool has_damper, bool is_front ) override;

   private:
       void updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep) override;
       void debugDraw(btIDebugDraw* debugDrawer) override;
   private:
	   void set_steer   (double steer) override;
	   void set_brake   (double brake) override;
       double get_steer () override;

   // rigid_body_impl
   private:
	   bt_rigid_body_ptr get_body() const;
	   void pre_update(double dt);
	   void has_contact(rigid_body_user_info_t const* other, osg::Vec3 const& local_point, osg::Vec3 const& vel);

   private:
       compound_shape_proxy                   chassis_shape_;
       rigid_body_proxy                       chassis_;
       raycast_vehicle_proxy                  raycast_veh_;
       polymorph_ptr<rigid_body_user_info_t>  self_;
       btRaycastVehicle::btVehicleTuning      tuning_;
	   system_impl_ptr						  sys_;

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
	   std::vector<size_t> wheels_ids_;
   };


}
}