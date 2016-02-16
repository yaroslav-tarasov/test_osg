#pragma once

#include "human_view.h"
// #include "phys/phys_sys.h"
#include "common/phys_sys.h"
#include "common/phys_object_model_base.h"
//#include "objects/ani.h"
#include "network/msg_dispatcher.h"

#include "human_common.h"
#include "human_model_states.h"
#include "common/aircraft.h"
#include "common/airports_manager.h"
#include "common/stdrandgen.h"

namespace human
{

struct model
      : model_info                // интерфейс информации о модели
      , model_control             // интерфейс управления моделью
      , view
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

    
    // model_info
private:
    // phys::rigid_body_ptr get_rigid_body() const;
    geo_position         get_phys_pos  () const;


    // model_control
private:
    void                 set_desired     ( double time,const cg::point_3& pos, const cg::quaternion& q, const double speed );
    void                 set_ext_wind    ( double speed, double azimuth );

public:
    void set_max_speed(double max_speed);
    cg::geo_point_2 phys_pos() const;

    nodes_management::node_info_ptr get_root();

private:
    void on_go_to_pos(msg::go_to_pos_data const& data);
    void on_follow_route(uint32_t route_id);

    void on_follow_trajectory(uint32_t /*route_id*/);

public:
    fms::trajectory_ptr  get_trajectory();

private:
    void follow_route(std::string const& route);
    void detach_cur_route();

    void on_zone_created( size_t id );
    void on_zone_destroyed( size_t id );
    void create_phys();
    void update_model( double dt );
    void sync_phys(double dt);
    void sync_nodes_manager( double dt );
    void settings_changed();

private:
    void go(cg::polar_point_2 const &dir) ;

private:
	void idle();
	void walk();
	void run();

private:
    //PY_REG_STRUCT()
    //{
    //    using namespace py;

    //    class_<model, bases<base_view_presentation>, noncopyable>("human", py::no_init)
    //        .def("go",         &model::go)
    //        .def("attach_tow", &model::on_attach_tow)
    //        .def("detach_tow", &model::on_detach_tow);
    //}


private:
    airports_manager::info_ptr airports_manager_;
    airport::info_ptr          airport_;


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


    model_state_ptr      model_state_; 

	//  phys staff
private:
	model_system *                  sys_;
	optional<double>                last_update_;
	double max_speed_;
	phys::character::info_ptr       phys_model_;
	optional<size_t>                phys_zone_;

	cg::geo_point_3                 desired_position_;
	quaternion                      desired_orien_;

private:
    double steer_course ;
    
	std_simple_randgen                      rnd_;

	enum anim_state
	{
		as_idle,
		as_walk,
		as_run
	};
	
	float                               _targetSpeed;

	anim_state          anim_state_;

	bool               start_follow_;
};

} // human
