#include "stdafx.h"
#include "precompiled_objects.h"

#include "arresting_gear_view.h"

namespace arresting_gear
{


object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(arresting_gear_view, view::create);


view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{

    if (nodes_manager_ = find_first_child<nodes_management::manager_ptr>(this))
    {
        root_ = nodes_manager_->get_node(0);
        conn_holder() << nodes_manager_->subscribe_model_changed(boost::bind(&view::on_model_changed_internal, this));
    }

    msg_disp()
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1))
        .add<msg::ropes_state> (boost::bind(&view::on_ropes_state, this, _1))
        .add<msg::target_msg>  (boost::bind(&view::on_set_target, this, _1))
        
         
        ; 
}

geo_point_3 view::pos() const
{
    return state_.pos;
}

std::string const& view::name() const
{
    return settings_.model;
}

settings_t const& view::settings() const
{
    return settings_;
}

ropes_state_t const& view::ropes_state() const
{
    return ropes_state_;
}

void view::on_settings(msg::settings_msg const& msg)
{

	settings_ = msg;
    on_new_settings();
}

void view::on_ropes_state(msg::ropes_state const& msg)
{
    ropes_state_ =  std::move(msg.state);

    on_new_ropes_state();
}

void view::on_set_target( boost::optional<uint32_t> id)
{
    auto old_target = target_id_;
	//target_ = id ? collection_->get_object(*id) : nullptr;
    target_id_ = id;

    on_target_changed(old_target, id) ;
}


void view::on_model_changed_internal()
{
#if 0
    if (nodes_manager_)
    {   
        if (nodes_manager_->get_model() != get_model(name()))
            on_model_changed();
    }
#endif
}

void view::update(double time)
{
}



} // arresting_gear 


