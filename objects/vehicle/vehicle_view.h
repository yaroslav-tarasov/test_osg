#pragma once

#include "vehicle_common.h"
#include "vehicle_msg.h"
// #include "common/radar.h"
#include "objects/nodes_management.h"
#include "common/aircraft.h"



namespace vehicle
{
    
//! ������������� ������ �� ����������
struct vehicle_data
{
    vehicle_data()
    {
    }

    vehicle_data(settings_t const& settings, state_t const& state)
        : settings_(settings)
        , state_   (state  )
    {
    }

protected:
    settings_t settings_;
    state_t    state_;

    REFL_INNER(vehicle_data)
        REFL_ENTRY(settings_)
        REFL_ENTRY(state_)
    REFL_END()
};

//! ���
struct view
    : base_view_presentation            //! ������� ���
    , vehicle_data
    //, obj_data_holder<vehicle_data>     //! ������������� ������ �� ����������
{
    static object_info_ptr create(kernel::object_create_t const& oc/*, dict_copt dict*/);

protected:
    view( kernel::object_create_t const& oc/*, dict_copt dict*/);

    // base_view_presentation
protected:
    void on_object_destroying(object_info_ptr object) override;
    void on_child_removing(kernel::object_info_ptr child) override;

protected:
    void on_state   (state_t const& state);
    void on_settings(settings_t const& settings);
    void on_tow     (optional<uint32_t> id);
    void on_model_changed();

public:
    geo_point_2 const& pos() const {return state_.pos;}
    double course() const {return state_.course;}
    double speed() const {return state_.speed;}
    point_2 dpos() const {return point_2(cg::polar_point_2(1., state_.course)) * state_.speed;}

protected:
    virtual void on_state_changed() {}
    virtual void settings_changed() {}
    virtual void on_aerotow_changed(aircraft::info_ptr old_aerotow) {}

public:
    void set_settings( settings_t const& settings );
    void set_state(state_t const&state);
    void set_tow( optional<uint32_t> tow_id );

protected:
    nodes_management::manager_ptr       nodes_manager_;
    nodes_management::node_control_ptr  root_;
    nodes_management::node_info_ptr     tow_point_node_;
    aircraft::info_ptr                  aerotow_;

private:
    using vehicle_data::state_;
};

} // end of vehicle