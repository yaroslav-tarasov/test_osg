#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_child_view.h"

namespace flock
{

namespace child
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(flock_child_view, view::create);


view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
    , _spawner              (find_first_object<manager::info_ptr>(collection_))
    , _init                 (_spawner._empty())
{

    if (nodes_manager_ = find_first_child<nodes_management::manager_ptr>(this))
    {
        root_ = nodes_manager_->get_node(0);
        conn_holder() << nodes_manager_->subscribe_model_changed(boost::bind(&view::on_model_changed_internal, this));
    }

    msg_disp()
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1));
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

void view::on_settings(msg::settings_msg const& msg)
{
#if 0  // Нужно или нет пока не понятно 
	if (nodes_manager_ && nodes_manager_->get_model() != settings.model)
		nodes_manager_->set_model(settings.model);
#endif
	
	settings_ = msg.settings;
    on_new_settings();
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
    if (!_init)
    {
        _spawner = find_first_object<manager::info_ptr>(collection_);
        _init = _spawner._empty();
    }
}

void view::on_child_removing(object_info_ptr child)
{
    base_view_presentation::on_child_removing(child) ;

    if (nodes_manager_ == child)
        nodes_manager_.reset() ;
}

}

} // flock 


