#include "stdafx.h" 
// FIXME
#include "precompiled_objects.h"

#include "vehicle_view.h"

namespace vehicle
{
view::view( kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{
    if (dict)
        set_name(oc.name);

    nodes_manager_ = find_first_child<nodes_management::manager_ptr>(this);
    if (nodes_manager_)
    {
        root_ = nodes_manager_->get_node(0);
        conn_holder()    << nodes_manager_->subscribe_model_changed(boost::bind(&view::on_model_changed, this));
        tow_point_node_  = nodes_manager_->find_node("tow_point");
        rtow_point_node_ = nodes_manager_->find_node("rtow_point");
    }

    msg_disp()
        .add<msg::state_msg_t     >(boost::bind(&view::on_state   , this, _1))
        .add<msg::settings_msg_t  >(boost::bind(&view::on_settings, this, _1))
        .add<msg::tow_msg_t       >(boost::bind(&view::on_tow     , this, _1))

        .add<msg::traj_assign_msg >(boost::bind(&view::on_traj_assign, this, _1))
        ;
}
    
object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(vehicle_view, view::create);

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object);

    if (object == aerotow_)
    {
        auto old_aerotow = aerotow_;
        aerotow_.reset();
        on_aerotow_changed(old_aerotow, boost::none ) ;
    }

}

void view::on_child_removing(object_info_ptr child)
{
    base_view_presentation::on_child_removing(child);

    if (child == nodes_manager_)
        nodes_manager_.reset();
}

void view::on_state(state_t const& state)
{                                              
    state_ = state;
    on_state_changed();     // Задействован только в чарте
}

void view::on_settings(settings_t const& settings)
{
    if (nodes_manager_ && nodes_manager_->get_model() != settings.model)
        nodes_manager_->set_model(settings.model);

    settings_ = settings;
    settings_changed();
}

void view::on_tow(/*optional<uint32_t> id*/msg::tow_msg const& msg)
{
    auto old_aerotow = aerotow_;
    aerotow_ = msg.tow_id ? collection_->get_object(*msg.tow_id) : nullptr;

    on_aerotow_changed(old_aerotow, msg) ;
}

void view::on_model_changed()
{
    root_ = nodes_manager_->get_node(0);
	tow_point_node_  = nodes_manager_->find_node("tow_point");
	rtow_point_node_ = nodes_manager_->find_node("rtow_point");
}

void view::set_settings( settings_t const& settings )
{
    set(msg::settings_msg_t(settings));
}

void view::set_state(state_t const& state)
{
    set(msg::state_msg_t(state), false);
}

void view::set_tow(optional<uint32_t> tow_id, bool reverse, const geo_position& pos)
{
    set(msg::tow_msg_t(tow_id, reverse, pos), true);
}

void view::on_traj_assign(msg::traj_assign_msg const &m)
{
    double len =0;
    if(traj_.get())
        len = traj_->cur_len();

    traj_ = fms::trajectory::create(m.traj);
    traj_->set_cur_len(len);

}

}


