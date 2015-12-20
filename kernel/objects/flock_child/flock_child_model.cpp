#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_child_model.h"
//#include "common/collect_collision.h"
#include "phys/sensor.h"

namespace flock
{

namespace child
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

AUTO_REG_NAME(flock_child_model, model::create);

model::model( kernel::object_create_t const& oc, dict_copt dict )
    : view                  (oc, dict)
    , phys_object_model_base(collection_)
    , nodes_manager_        (find_first_child<nodes_management::manager_ptr>(this))
{
    create_phys();
}

void model::update(double time)
{
    view::update(time);
}

void model::on_child_removing(object_info_ptr child)
{
    view::on_child_removing(child);

    if (nodes_manager_ == child)
        nodes_manager_.reset();
}


void model::create_phys()
{
    if (!phys_)
         return;
    
}

}

} // end of flock
