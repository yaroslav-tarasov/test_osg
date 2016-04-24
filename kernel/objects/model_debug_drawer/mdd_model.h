#pragma once

#include "mdd_view.h"
#include "common/model_debug_drawer.h"
#include "common/phys_object_model_base.h"


namespace mdd
{

    struct model
        : model_presentation
        , view
        , info
        , phys_object_model_base    
    {
		friend struct remote_render;

        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        model(kernel::object_create_t const& oc, dict_copt dict);

        // base_presentation
    private:
        void update( double /*time*/ );

        // info
    private:
        debug_render_ptr get_renderer();

    private:
        void render();
    
    private:
        debug_render_ptr renderer_;
		bool				 init_;
    };

}