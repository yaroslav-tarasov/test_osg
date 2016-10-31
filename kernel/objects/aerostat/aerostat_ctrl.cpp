#include "aerostat_ctrl.h"

namespace aerostat
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}
AUTO_REG_NAME(aerostat_ext_ctrl, ctrl::create);

ctrl::ctrl(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
    , anim_started(false)
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

    if(time>10.0 && !anim_started)
    {
        anim_started = true;
        // auto const& settings = _spawner->settings();
        
        // root_->play_animation("flap", 1.0 / rnd_.random_range(settings._minAnimationSpeed, settings._maxAnimationSpeed), -1., -1., 0.0);
    }
}



} // aerostat 


