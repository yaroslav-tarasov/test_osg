#pragma once

#include "flock_manager_view.h"

#include "common/phys_sys.h"
#include "common/phys_object_model_base.h"

namespace flock
{

	namespace manager
	{

		struct model
			: view 
            , model_ext_control             // интерфейс управления моделью
            , phys_object_model_base  
		{
			static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

		protected:
			model(kernel::object_create_t const& oc, dict_copt dict);
            
            // base_presentation
        private:
            void update( double /*time*/ ) override;

            // model_control_ext
        private:
            void                 set_desired     ( double time,const cg::point_3& pos, const cg::quaternion& q, const double speed );
            void                 set_ext_wind    ( double speed, double azimuth );

        private:
            model_system *                   sys_;
            optional<double>                 last_update_;
            optional<size_t>                 phys_zone_;
		};


	}

}
