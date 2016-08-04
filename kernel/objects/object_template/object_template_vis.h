#pragma once

#include "arresting_gear_view.h"
#include "av/avLine/avRopes.h"


namespace arresting_gear
{

struct visual
    : view

{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    visual( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
private:
    void update( double /*time*/ ) override;

    // view
private:
    void on_new_ropes_state() override;

private:
    visual_object_ptr    ropes_object_;
    RopesNodePtr         ropes_weak_ptr_;


};



}