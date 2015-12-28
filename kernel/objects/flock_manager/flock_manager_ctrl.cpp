#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_manager_ctrl.h"

//       Для создания дочерних объектов (чет фуфел какой-то)
FIXME(Создание множества дочерних объектов и клонирование)
#include "kernel/systems/fake_system.h"
#include "flock_child/flock_child_view.h"
#include "nodes_manager/nodes_manager_view.h"


namespace flock
{

using namespace kernel;

namespace child 
{
    object_info_ptr create(fake_objects_factory* sys,const settings_t& sett,const geo_position& init_pos)
    {
        const std::string class_name = "flock_child";
        const std::string unique_name = sys->generate_unique_name(class_name);

        obj_create_data ocd(class_name, unique_name, dict::wrap(child_data(sett, state_t(init_pos.pos, init_pos.orien))));
        ocd
            .add_child(obj_create_data("nodes_manager", "nodes_manager", dict::wrap(nodes_management::nodes_data          ())));

        return sys->create_object(ocd);	
    }
}

namespace manager
{


object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new ctrl(oc, dict));
}


AUTO_REG_NAME(flock_manager_ext_ctrl, ctrl::create);

ctrl::ctrl( kernel::object_create_t const& oc, dict_copt dict)
    : view(oc,dict)
{
    auto * of = dynamic_cast<kernel::fake_objects_factory*>(sys_);
    
    settings_._childAmount = 70;
	
	simplerandgen  rnd(static_cast<unsigned>(time(nullptr)));

    child::settings_t vs;
    vs.model        = "crow"; 
	
	// transform.position = (Random.insideUnitSphere *_spawner._spawnSphere) + _spawner.transform.position;

    decart_position child_pos;

    for (int i=0; i < settings_._childAmount; ++i )
    {
        // child_pos.pos   = point_3(rnd.random_8bit(),rnd.random_8bit(),300 + rnd.random_8bit()/2.0);

        // transform.position = (Random.insideUnitSphere *_spawner._spawnSphere) + _spawner.transform.position;
		FIXME("Чудесный код, преобразование туда сюда")
        point_3 spawner_pos = cg::geo_base_3(get_base())(state_.pos);
        child_pos.pos   = point_3(rnd.random_range(-1.0,1.0), rnd.random_range(-1.0,1.0), rnd.random_range(-1.0,1.0) ) * settings_._spawnSphere + spawner_pos;
		child_pos.orien = state_.orien;
        geo_position vgp(child_pos, get_base());

        roamers_.insert(child::create(of,vs,vgp));
    }
}

} // manager

} // flock