#pragma once

#include "rocket_flare_view.h"


namespace rocket_flare
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
    void on_new_state() override;

private:
    visual_object_ptr    flare_object_;
    // RopesNodePtr         flare_weak_ptr_;


};



}