#pragma once 
#include "airport_view.h"
#include "network/msg_dispatcher.h"
#include "objects/impl/local_position.h"
#include "kernel/systems/vis_system.h"

namespace airport 
{

struct vis
    : view 
    , vis_info
    , visual_control
{
    static object_info_ptr create(object_create_t const& oc, dict_copt dict);

private:
    vis(object_create_t const& oc, dict_copt dict);

// vis_info
private:
    bool is_visible() const ;

private:
    void on_new_settings() override;
    void on_model_changed() override;

    void retreive_camera();
#if 0
    void place_lights   ();
    void place_marking  ();
#endif

protected:
    void update(double time);

private:
    geo_point_3 camera_pos  () const;
    cpr         camera_orien() const;
    double      zoom () const; 


private:
    geo_point_3 pos  () const override;
    cpr         orien() const override;

private:
    kernel::visual_system     * vis_sys_;

private:
#if 0
    vector<victory::node_ptr>   lamps_;
#endif
    geo_position                camera_pos_;
};

} // airport 