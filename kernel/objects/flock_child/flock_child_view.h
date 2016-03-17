#pragma once

#include "flock_child_common.h"
#include "common/flock_child.h"

#include "flock_manager/flock_manager_common.h"
#include "common/flock_manager.h"
#include "common/stdrandgen.h"


namespace flock
{

namespace child
{

struct child_data
{
    child_data()
    {
    }

    child_data(settings_t const& settings, state_t const& state)
        : settings_(settings)
        , state_   (state  )
    {
    }

protected:
    settings_t settings_;
    state_t    state_;

    REFL_INNER(child_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(state_)
    REFL_END()
};

struct view
    : base_view_presentation
    , obj_data_holder<child_data>
    , info
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
protected:
    void update(double time) override;
    
    // base_view_presentation
protected:
    void on_child_removing   (object_info_ptr child)  override ;
    void on_object_destroying( object_info_ptr object ) override;

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

    manager::info_wptr                   _spawner;
    bool                                 _init;

	float                               _targetSpeed;
};


}

}
