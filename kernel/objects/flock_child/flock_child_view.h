#pragma once

#include "flock_child_common.h"
#include "common/flock_child.h"


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

#if 0

    public var _spawner:FlockController;
    public var _wayPoint : Vector3;
    public var _speed:float= 10;
    public var _dived:boolean =true;
    public var _stuckCounter:float;			//prevents looping around a waypoint by increasing minimum distance to waypoint
    public var _damping:float;
    public var _soar:boolean = true;
    public var _flatFlyDown:boolean;
    public var _landing:boolean;
    public var _landingSpotted:boolean;
    private var _lerpCounter:int;
    public var _targetSpeed:float;
    public var _move:boolean = true;
    var        _model:GameObject;

    var        _avoidValue:float;		//Random value used to check for obstacles. Randomized to lessen uniformed behaviour when avoiding
    var        _avoidDistance:float;

    private var _soarTimer:float;	

#endif

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

};


}

}
