#pragma once

#include "vehicle_view.h"
// #include "phys/phys_sys.h"
#include "common/phys_sys.h"
#include "common/phys_object_model_base.h"
//#include "objects/ani.h"
#include "network/msg_dispatcher.h"

#include "vehicle_common.h"
#include "vehicle_model_states.h"
#include "common/aircraft.h"

namespace vehicle
{

struct model
      : model_base
      , view
    //, base_view_presentation
      , phys_object_model_base    
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

private:
    model(kernel::object_create_t const& oc, dict_copt dict);

    // base_presentation
private:
    void update( double /*time*/ ) override;

    // base_view_presentation
private:
    void on_object_created(object_info_ptr object) override;
    void on_object_destroying(object_info_ptr object) override;

    // view
private:
    void on_aerotow_changed(aircraft::info_ptr old_aerotow) override;

public:
    void set_max_speed(double max_speed);
    void set_course_hard(double course);
    cg::geo_point_2 phys_pos() const;

    nodes_management::node_info_ptr get_root();

private:
    void on_attach_tow( uint32_t tow_id );
    void on_detach_tow();
    void on_go_to_pos(msg::go_to_pos_data const& data);
    void on_follow_route(uint32_t route_id);
    //void on_debug_controls(msg::debug_controls_data const&);
    //void on_disable_debug_controls(msg::disable_debug_ctrl_msg_t const& d);

public:
    void go_to_pos(  cg::geo_point_2 pos, double course );

private:
    void follow_route(std::string const& route);
    void detach_cur_route();

    void update_model( double dt );
    void on_zone_created( size_t id );
    void on_zone_destroyed( size_t id );
    void create_phys_vehicle();
    void sync_phys();
    void sync_nodes_manager( double dt );
    void settings_changed();

private:
    void go(cg::polar_point_2 const &dir) ;

private:
    //PY_REG_STRUCT()
    //{
    //    using namespace py;

    //    class_<model, bases<base_view_presentation>, noncopyable>("vehicle", py::no_init)
    //        .def("go",         &model::go)
    //        .def("attach_tow", &model::on_attach_tow)
    //        .def("detach_tow", &model::on_detach_tow);
    //}


private:
    model_system *    sys_;
    optional<double> last_update_;
    double max_speed_;

    //ani_object::info_ptr    ani_ ;
    //ani::airport_info_ptr  airport_;

    phys::ray_cast_vehicle::info_ptr phys_vehicle_;
    optional<size_t> phys_zone_;

    cg::geo_base_3 root_next_pos_;
    cg::quaternion root_next_orien_;

    cg::transform_4 body_transform_inv_;

    struct wheel_t 
    {
        wheel_t (nodes_management::node_control_ptr wheel_node)
            : node(wheel_node)
        {}

        nodes_management::node_control_ptr node;
    };

    nodes_management::node_info_ptr body_node_;

    std::vector<wheel_t> wheels_;

    model_state_ptr      model_state_; // В оригинале state_ и мне это не нравиться

    bool                manual_controls_;

private:
    double rod_course ;
    double air_course ;
    double steer_course ;

// from view / data
public:
    // FIXME преносим обратно
    void set_state_debug(state_t const& state)
    {
        // И здесь тоже чего-то надо сделать
        set(msg::state_msg_t(state), false);
    }

};

} // vehicle
