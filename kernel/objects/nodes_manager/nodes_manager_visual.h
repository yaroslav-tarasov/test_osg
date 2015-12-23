#pragma once
#include "nodes_manager_view.h"
#include "kernel/kernel_fwd.h"

namespace nodes_management
{

struct visual
    : kernel::visual_presentation
    , view
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

private:
    visual(kernel::object_create_t const& oc, dict_copt dict);

public:
    void init() override;
    visual_object_ptr visual_object()    { return visual_object_; }
    kernel::visual_system * vis_system() { return sys_; }

private:
    void apply_model(string const& model);
    void apply_vis_model();


private:
    void object_loaded( uint32_t seed );

private:
    kernel::visual_system * sys_;
    visual_object_ptr visual_object_;
};

} // nodes_management
