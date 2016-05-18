#pragma once

#include "arresting_gear_common.h"
#include "arresting_gear_view.h"
#include "common/phys_sys.h"
#include "objects/nodes_management.h"
#include "common/phys_object_model_base.h"

namespace arresting_gear
{



struct model
    : model_presentation        // базовый класс, виртуальрость
    , view                      // базовое представление
    , phys_object_model_base    // базовый класс для объектов с физическими моделями
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

private:
    model( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
private:
    void update(double time) override;

    // base_view_presentation
private:
    void on_child_removing(object_info_ptr child) override;

private:
    
    void create_phys        ();
    void sync_phys          ( double dt );
    void sync_nodes_manager ( double dt );
    void update_model       ( double time, double dt );

private:
    phys::static_mesh_ptr                  mesh_;

    optional<size_t>                  phys_zone_;

//  phys staff
private:
    model_system *                           sys_;
	optional<double>                 last_update_;
	phys::arresting_gear::info_ptr      phys_model_;
    
    cg::geo_point_3              desired_position_;
    quaternion                      desired_orien_;

};


} // end of namespace model
