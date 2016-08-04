#pragma once

#include "rocket_flare_common.h"
#include "common/rocket_flare.h"

#include "common/stdrandgen.h"


namespace rocket_flare
{

struct rocket_flare_data
{
    rocket_flare_data()
    {
    }

    rocket_flare_data(settings_t const& settings, state_t const& state)
        : settings_(settings)
        , state_   (state  )
    {
    }

protected:
    settings_t settings_;
    state_t    state_;

    REFL_INNER(rocket_flare_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(state_)
    REFL_END()
};

struct view
    : base_view_presentation
    , obj_data_holder<rocket_flare_data>
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
    settings_t    const& settings() const;
    ropes_state_t const& ropes_state() const;

private:
    void on_settings              ( msg::settings_msg const& msg );
    void on_model_changed_internal();
    void on_ropes_state           ( msg::ropes_state const& msg );
private:
    virtual void on_new_settings(){}
    virtual void on_model_changed(){}
    virtual void on_new_state(){}
protected:
    nodes_management::manager_ptr       nodes_manager_;
    nodes_management::node_control_ptr  root_;

   	//aircraft::info_ptr                     target_;
	 boost::optional<uint32_t>            target_id_;
	     std_simple_randgen                      rnd_;
private:
    ropes_state_t                       ropes_state_;

};




}
