#pragma once

#include "flock_child_common.h"
#include "flock_child_view.h"
#include "flock_manager/flock_manager_common.h"
#include "common/flock_manager.h"
#include "common/phys_sys.h"
#include "objects/nodes_management.h"
#include "common/phys_object_model_base.h"

namespace flock
{

namespace child
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
    void create_phys();
	void flap();

private:
    nodes_management::manager_ptr nodes_manager_;

    phys::static_mesh_ptr                  mesh_;

    optional<size_t>                  phys_zone_;

    manager::info_ptr                   _spawner;

	// Some data
private:

	bool									_soar; // state ?
	bool								   _dived;

	double  						   _soarTimer;


private:
	optional<double>                 last_update_;

};

} // child

} // end of namespace flock
