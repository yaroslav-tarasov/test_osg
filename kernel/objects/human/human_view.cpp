#include "human_view.h"

namespace human
{
    
object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(human_view, view::create);

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
		conn_holder() << nodes_manager_->subscribe_model_changed(boost::bind(&view::on_model_changed, this));
	}

	msg_disp()
		.add<msg::state_msg_t   >(boost::bind(&view::on_state   , this, _1))
		.add<msg::settings_msg_t>(boost::bind(&view::on_settings, this, _1))

		.add<msg::traj_assign_msg   >(boost::bind(&view::on_traj_assign, this, _1))
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


void view::on_model_changed()
{
    root_ = nodes_manager_->get_node(0);
}

void view::set_settings( settings_t const& settings )
{
    set(msg::settings_msg_t(settings));
}

void view::set_state(state_t const& state)
{
    set(msg::state_msg_t(state), false);
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


