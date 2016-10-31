#include "rocket_flare_ctrl.h"

namespace rocket_flare
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}

AUTO_REG_NAME(rocket_flare_ext_ctrl, ctrl::create);

ctrl::ctrl(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
{
    
    if (nodes_manager_)
    {
        if (nodes_manager_->get_model() != settings_.model)
            nodes_manager_->set_model(settings_.model);
         
        root_->set_position(geo_position(state_.pos, point_3(), state_.orien, point_3()));

    }


}

void ctrl::update( double time )
{   
    view::update(time);
}


} // rocket_flare 


