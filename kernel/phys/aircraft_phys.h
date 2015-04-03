#pragma once

#include "rigid_body_info.h"
#include "cpp_utils/polymorph_ptr.h"
#include "phys_sys.h"
#include "phys_sys_common.h"
#include "aircraft.h"
#include "sensor.h"


namespace phys
{
	namespace aircraft
	{

   
   // ATTRIBUTE_ALIGNED16 (class impl)
   class impl
       : public rigid_body_user_info_t
	   , public rigid_body_impl
       , public btActionInterface
	   , public control
   {
   public:
      impl(system_impl_ptr,compound_sensor_ptr s,/*compound_shape_proxy& s,*/ params_t const& params, decart_position const& pos);
   
   
   public:
	   size_t add_wheel   ( double mass, double width, double radius, point_3 const& offset, cpr const & orien, bool has_damper, bool is_front ) override;
       void   remove_wheel( size_t id );

   private:
       void updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep) override;
       void debugDraw(btIDebugDraw* debugDrawer) override;
   
   //info
   private:
       decart_position get_position() const;
       decart_position get_wheel_position( size_t i ) const ;
       double Ixx() const ;
       double Iyy() const ;
       double Izz() const ;
       params_t const& params() const ;
       double drag() const;
       double lift() const;
       double thrust() const ;
       bool has_contact() const;
       std::vector<contact_info_t> get_body_contacts() const;
       bool has_wheel_contact(size_t id) const;
       double wheel_skid_info(size_t id) const;

   private:
	   void   set_steer   (double steer)            override;
	   void   set_brake   (double brake)            override;
       double get_steer   ()                        override;
       void   set_thrust  (double thrust)           override;
       void   set_elevator(double elevator)         override;
       void   set_ailerons(double ailerons)         override;
       void   set_rudder  (double rudder)           override;
       void   set_wind    (cg::point_3 const& wind) override;
       void   apply_force (point_3 const& f)        override;

       void   update_aerodynamics(double dt)    override;
       void   reset_suspension()                override;

   // rigid_body_impl
   private:
	   bt_rigid_body_ptr get_body() const;
	   void pre_update(double dt);
	   void has_contact(rigid_body_user_info_t const* other, cg::point_3 const& local_point, cg::point_3 const& vel);

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

       cg::point_3  wind_;

       double   prev_attack_angle_;

       bool     has_chassis_contact_;

       typedef cg::duplicate_points_fixed_id<cg::point_3> dup_points_t;
       dup_points_t  body_contact_points_;

       struct contact_t
       {
           contact_t() {}
           contact_t(cg::point_3 const& vel)
               : sum_vel(vel)
               , count(1)
           {}

           cg::point_3 sum_vel;
           size_t count;
       };

       fixed_id_vector<contact_t> body_contacts_;

       fixed_id_vector<size_t> wheels_ids_;
   };


}
}
