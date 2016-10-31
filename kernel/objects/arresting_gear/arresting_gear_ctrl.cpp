#include "arresting_gear_ctrl.h"

namespace arresting_gear
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}
AUTO_REG_NAME(arresting_gear_ext_ctrl, ctrl::create);

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

void ctrl::set_target(const boost::optional<uint32_t>&   target_id)
{
      set(msg::target_msg(target_id));
}

} // arresting_gear 


