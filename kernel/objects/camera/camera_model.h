#pragma once 
#include "camera_view.h"
#include "network/msg_dispatcher.h"

namespace camera_object
{
	struct model
		: visual_presentation
		, model_ext_control
		, view
	{
		static object_info_ptr create(object_create_t const& oc, dict_copt dict);

	private:
		model(object_create_t const& oc, dict_copt dict);
             
        void update      ( double time  );
        void sync_traj   ( double time  );
        void sync_nm_root( double dt    );

    private:
        void  set_desired   (double time,const cg::point_3& pos, const cg::quaternion& q, const double speed);

    private:
        optional<geo_point_3>                  desired_nm_pos_;
        optional<quaternion>                   desired_nm_orien_;

        optional<double>                       last_update_;
        
        double                                 nm_ang_smooth_;

    private:
        fms::trajectory_ptr                         traj_;

	};

}