#pragma once 
#include "camera_common.h"
#include "objects/nodes_management.h"
#include "common/camera_common.h"

namespace camera_object
{

struct view
    : base_view_presentation        
    , obj_data_holder<camera_data>
    , info
{
    static object_info_ptr create(object_create_t const& oc, dict_copt dict);

public:
    geo_point_3 pos  () const;  
    cpr         orien() const;  

protected:
    view(kernel::object_create_t const& oc, dict_copt dict);

private:
    void on_child_removing(kernel::object_info_ptr child) override;
    
protected:
    nodes_management::manager_ptr       mng () const;
    nodes_management::node_control_ptr  root() const;

protected:
    binoculars_t const& binoculars() const;
    virtual void on_new_binoculars() {}

private:
    void on_binoculars(msg::binoculars_msg const&);

private:
    void binocular_on(unsigned target_id, double zoom);
    void binocular_off();

private:
    //PY_REG_STRUCT()
    //{
    //    using namespace py;

    //    class_<view, bases<base_view_presentation>, noncopyable>("view", py::no_init)
    //        .def("binocular_on",  &view::binocular_on)
    //        .def("binocular_off", &view::binocular_off)
    //        ;
    //}

private:
    nodes_management::manager_ptr mng_;

protected:
	fms::trajectory_ptr                         traj_;
};

} // camera_object
