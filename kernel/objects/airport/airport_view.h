#pragma once

#include "airport_common.h"
#include "airport_msg.h"
#include "common/airport.h"

#include "atc/airport.h"

namespace airport
{

struct port_data
{   
    port_data()
    {
    }

    port_data(settings_t const& settings, atc::airport::data_t const& data)
        : settings_(settings)
        , data_    (data  )
        {
        }

protected:
    settings_t           settings_;
    atc::airport::data_t data_;

    REFL_INNER(port_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(data_)
    REFL_END()
};

struct view
    : base_view_presentation
    , obj_data_holder<port_data>
    , info
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view( kernel::object_create_t const& oc, dict_copt dict );

//info
protected:
    geo_point_3        pos () const;
    std::string const& name() const;

protected:
    settings_t const& settings() const;
    void import_ani();

private:
    void on_settings(msg::settings_msg const& msg);
    void on_model_changed_internal();
    void on_changed_pov(msg::changed_pov_msg_t const& msg);
private:
    virtual void on_new_settings(){}
    virtual void on_new_pov(uint32_t old_pov){}
    virtual void on_model_changed(){}
private:
    nodes_management::manager_ptr       nodes_manager_;

protected:
 	uint32_t                           current_camera_;
};

}
