#pragma once


#include "flock_manager_common.h"
#include "common/flock_child.h"
#include "common/flock_manager.h"
#include "flock_manager_msg.h"


namespace flock
{

	namespace manager
	{

        struct manager_data
        {
            manager_data()
            {
            }

            manager_data(settings_t const& settings, state_t const& state)
                : settings_(settings)
                , state_   (state  )
            {
            }

        protected:
            settings_t settings_;
            state_t    state_;

            REFL_INNER(manager_data)
                REFL_ENTRY(settings_)
                REFL_ENTRY(state_)
           REFL_END()
        };


		struct view
			: base_view_presentation
			, obj_data_holder<manager_data>
			, info
		{
			static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

		protected:
			view(kernel::object_create_t const& oc, dict_copt dict);

			// base_view
		protected:
			void on_object_created(object_info_ptr object)    override;
			void on_object_destroying(object_info_ptr object) override;
			// info
			settings_t  const& settings()               const override;
			geo_point_3 const& pos()                    const override;
        
        protected:
            virtual void on_state_changed() {}    // Задействован только в чарте
        
        protected:
            void on_state   (state_t const& state);

        public:
            //void set_settings( settings_t const& settings );
            void set_state(state_t const&state);

		protected:
			std::set<child::info_ptr>				 roamers_;

        protected:
            fms::trajectory_ptr                         traj_;
		};


	}

}
