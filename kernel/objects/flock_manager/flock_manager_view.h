#pragma once


#include "flock_manager_common.h"
#include "common/flock_child.h"
#include "common/flock_manager.h"

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
			const settings_t& settings()                 const override;
		protected:
			std::set<child::info_ptr>				       	roamers_;

		};


	}

}
