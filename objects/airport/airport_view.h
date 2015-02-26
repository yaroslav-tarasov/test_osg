#pragma once

#include "airport_common.h"
#include "common/airport.h"

#include "atc/airport.h"

namespace airport
{

struct port_data
{
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
private:
    virtual void on_new_settings(){}
    virtual void on_model_changed(){}
private:
    nodes_management::manager_ptr       nodes_manager_;

};

}
