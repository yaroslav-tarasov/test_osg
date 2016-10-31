#pragma once

#include "rocket_flare_view.h"

namespace visual_objects
{
    struct fire_trace_support;
}

#include "av/Fx.h"

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
    void on_new_contact_effect(double time, std::vector<contact_t> const& contacts) override;

private:
    // visual_object_ptr    flare_object_;
    visual_system*                                         vsys_;

    boost::shared_ptr<visual_objects::fire_trace_support>     fs_;


};



}