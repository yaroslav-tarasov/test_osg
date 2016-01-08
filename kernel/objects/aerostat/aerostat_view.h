#pragma once

#include "aerostat_common.h"
#include "common/aerostat.h"

#include "common/stdrandgen.h"


namespace aerostat
{

struct aerostat_data
{
    aerostat_data()
    {
    }

    aerostat_data(settings_t const& settings, state_t const& state)
        : settings_(settings)
        , state_   (state  )
    {
    }

protected:
    settings_t settings_;
    state_t    state_;

    REFL_INNER(aerostat_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(state_)
    REFL_END()
};

struct view
    : base_view_presentation
    , obj_data_holder<aerostat_data>
    , info
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
protected:
    void update(double time) override;

    //info
protected:
    geo_point_3        pos () const;
    std::string const& name() const;

protected:
    settings_t const& settings() const;

private:
    void on_settings(msg::settings_msg const& msg);
    void on_model_changed_internal();
private:
    virtual void on_new_settings(){}
    virtual void on_model_changed(){}

protected:
    nodes_management::manager_ptr       nodes_manager_;
    nodes_management::node_control_ptr  root_;

    std_simple_randgen                      rnd_;

	float                               _targetSpeed;
};




}
