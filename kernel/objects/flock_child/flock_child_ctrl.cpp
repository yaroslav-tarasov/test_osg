#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_child_ctrl.h"


namespace flock
{

namespace child
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}
AUTO_REG_NAME(flock_child_ext_ctrl, ctrl::create);

ctrl::ctrl(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
    , anim_started(false)
{
    //nodes_management::manager_ptr manager = find_first_child<nodes_management::manager_ptr>(this);
    
    if (nodes_manager_)
    {
        if (nodes_manager_->get_model() != settings_.model)
            nodes_manager_->set_model(settings_.model);

        root_->set_position(geo_position(geo_point_3(pos(), 0), point_3(), quaternion(cpr(/*course()*/90, 0, 0)), point_3()));
        

    }


}

void ctrl::set_model(const std::string&  icao_code)
{
#if 0
    if (!valid_icao(icao_code))
        return;

    settings_t st;
    st.icao_code = icao_code;

    set(msg::settings_msg(st));

    nodes_management::manager_ptr manager = find_first_child<nodes_management::manager_ptr>(this);
    Assert(manager);

    if (manager->get_model() != get_model(name()))
    {
        std::string new_model = get_model(icao_code);
        manager->set_model(new_model);
        nodes_management::node_control_ptr root = manager->get_node(0);

        // root->set_position(geo_position(*atc::airport::kta_position(settings_.icao_code), point_3(), quaternion(), point_3()));
        // base_chart_t::set_position(app::layer_point_3(0, *atc::airport::kta_position(settings_.icao_code)));
        // update_visibility();
    }
#endif

}

void ctrl::update( double time )
{   
    view::update(time);

    if(time>10.0 && !anim_started)
    {
        anim_started = true;
        root_->play_animation("clip1", 5, -1., -1.);
    }
}

} // child 


} // flock 


