#include "stdafx.h"
#include "precompiled_objects.h"

#include "mdd_model.h"

namespace mdd
{
    object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
    {
        return object_info_ptr(new model(oc,dict));
    }

    model::model(kernel::object_create_t const& oc, dict_copt dict)
        : view(oc, dict)
        , phys_object_model_base(collection_)
    {          

    }

    void model::update( double /*time*/ )
    {
        optional<size_t> phys_zone = phys_->get_zone(::get_base());
        if(phys_zone )
        {
            auto phys_sys = phys_->get_system(*phys_zone);
            phys_sys->set_debug_renderer(renderer_);
        }
            
    }

    debug_render_ptr model::get_renderer()
    {
        return renderer_;
    }



    AUTO_REG_NAME(mdd_model, model::create);

}