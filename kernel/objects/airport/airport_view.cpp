#include "airport_view.h"
#include "objects/ani.h"
//#include "db_connector/db_connector.h"
#include "atc_data/atc_data.h"

namespace airport
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}
AUTO_REG_NAME(airport_view, view::create);


view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
    , current_camera_       (0)
{

    if (nodes_manager_ = find_first_child<nodes_management::manager_ptr>(this))
    {
        conn_holder() << nodes_manager_->subscribe_model_changed(boost::bind(&view::on_model_changed_internal, this));
    }

    msg_disp()
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1))
        .add<msg::changed_pov_msg_t>(boost::bind(&view::on_changed_pov , this, _1));

}

geo_point_3 view::pos() const
{
    FIXME(fake position)  
    return ::get_base();//*atc::airport::kta_position(settings_.icao_code);
}

std::string const& view::name() const
{
    return settings_.icao_code;
}

settings_t const& view::settings() const
{
    return settings_;
}

void view::on_settings(msg::settings_msg const& msg)
{
    settings_ = msg.settings;
    on_new_settings();
}

void view::on_changed_pov(msg::changed_pov_msg_t const& msg)
{
    uint32_t  old_pov = current_camera_;
    current_camera_ = msg;
    on_new_pov(old_pov);
}

void view::on_model_changed_internal()
{
    if (nodes_manager_)
    {   
        if (nodes_manager_->get_model() != get_model(name()))
            on_model_changed();
    }
}

// Import structure
void view::import_ani()
{
#if 0
    atc_data::client_interface_ptr db = atc_data::create_client();
    if (!db)
    {
        LogError("No atc data client");
        return;
    }

    data_ = db->get_airport_data(settings_.icao_code);

    if (data_.empty())
        return ;

    ani_object::control_ptr ani_control = find_first_object<ani_object::control_ptr>(collection_);
    if(!ani_control)
        return;

    ani_control->add_airport(settings_.icao_code, data_);

#endif
}

} // airport 


