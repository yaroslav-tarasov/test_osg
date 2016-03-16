#pragma once

#include "flock_manager_fwd.h"

namespace flock
{
	namespace manager
	{
		struct info
		{
			virtual geo_point_3 const&     pos() const =0;
			virtual const settings_t& settings() const =0;
			virtual ~info() {}
		};

		typedef polymorph_ptr<info>     info_ptr;
        typedef boost::weak_ptr<info>  info_wptr;

        struct model_ext_control
        {
            virtual ~model_ext_control() {}
            virtual void                 set_desired   (double time,const cg::point_3& pos, const cg::quaternion& q, const double speed)     = 0;
            virtual void                 set_ext_wind  (double speed, double azimuth) =0;
        };
	}

}