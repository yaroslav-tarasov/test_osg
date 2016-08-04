#pragma once

#include "cpp_utils/polymorph_ptr.h"
#include "sensor.h"


namespace phys
{
	namespace rocket_flare
	{
   // ATTRIBUTE_ALIGNED16 (class impl)
   class impl
       : public bt_body_user_info_t
	   , public rigid_body_impl
       , public btActionInterface
       , public info
	   , public control
   {
   public:
      impl(system_impl_ptr,compound_sensor_ptr s, params_t const& params, decart_position const& pos);
      ~impl();
   
   private:
       void updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep) override;
       void debugDraw(btIDebugDraw* debugDrawer) override;
   
   //info
   private:
       decart_position get_position() const override;
       params_t const& params() const ;
       bool has_contact() const;
       std::vector<contact_info_t> get_body_contacts() const;
                          

   // control
   private:
       void   set_wind    (cg::point_3 const& wind)    override;
       void   apply_force (point_3 const& f)           override;

       void   set_position         (const decart_position& pos) override;
       void   set_linear_velocity  (point_3 const& v)           override;
       void   set_angular_velocity (point_3 const& a)           override;


   // rigid_body_impl
   private:
	   bt_rigid_body_ptr get_body   () const;
	   void              pre_update (double dt);
	   void              has_contact(bt_body_user_info_t const* other, cg::point_3 const& local_point, cg::point_3 const& vel);
   
   private:
	   	void             update(double dt);
   private:
#if 0
       compound_shape_proxy                   chassis_shape_;
#else 
       btCompoundShape*                       chassis_shape_;
#endif

	   rigid_body_proxy                       chassis_;
       polymorph_ptr<bt_body_user_info_t>                         self_;
	   system_impl_ptr						                      sys_;
       params_t                                                   params_;
       cg::point_3                                                wind_;
       bool                                                       has_chassis_contact_;
       cg::point_3                                                self_offset_ ;

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

       fixed_id_vector<contact_t>        body_contacts_;
       cg::rectangle_3f                  trap_bound_;
	   rigid_body_ptr                    target_;

       boost::optional<double>           time_to_release;
   };


}
}
