#pragma once 
#include "camera_view.h"
#include "network/msg_dispatcher.h"

namespace camera_object
{
//! визуализация камеры
struct vis
    : visual_presentation
    , visual_control
    , view
{
    static object_info_ptr create(object_create_t const& oc, dict_copt dict);

private:
    vis(object_create_t const& oc, dict_copt dict);

    // base_presentation
private:
    void update(double time) override;

    // base_view_presentation
private:
    void on_object_destroying(kernel::object_info_ptr object) override;

    // visual_control
private:
    geo_point_3 pos  () const override;
    cpr         orien() const override;

private:
    void on_new_binoculars() override;

private:
    object_info_ptr                    track_obj_;
    nodes_management::node_control_ptr track_obj_root_;

private:
    double magnification_;

#if 0
private:
    victory::IBinocularPtr  binoculars_;
#endif
};

}