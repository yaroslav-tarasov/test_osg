#include "stdafx.h"
#include "precompiled_objects.h"

#include "airport_model.h"
//#include "common/collect_collision.h"
#include "phys/sensor.h"

namespace airport
{

object_info_ptr model::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new model(oc,dict));
}

AUTO_REG_NAME(airport_model, model::create);

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

void model::on_zone_created( size_t id )
{
    if (phys_->zone_name(id) == settings_.icao_code)
    {                         
        create_phys();
    }
}

void model::on_zone_destroyed( size_t /*id*/ )
{
    mesh_.reset();
    masts_.clear();
}


void model::create_phys()
{
    if (!phys_)
         return;
    FIXME(физика аэропорта настаивает на реализации)

    optional<size_t> zone = phys_->get_zone(settings_.icao_code);
    if (zone)
    {

        auto psys = phys_->get_system(*zone);
        geo_base_3 base = phys_->get_base(*zone);

        nm::node_info_ptr all_node = nodes_manager_->find_node("all");

        mesh_ = psys->create_static_mesh(phys::sensor_ptr());
#if 0
        if (all_node)
        {
            phys::sensor_ptr s = phys::get_sensor(*all_node->get_collision());
            if (s->chunks_count() > 0)
                mesh_ = psys->create_static_mesh(s);
        }


        nm::node_info_ptr masts_node = nodes_manager_->find_node("masts");
        if (masts_node)
        {
            nm::visit_sub_tree(nodes_manager_->get_node_tree_iterator(masts_node->node_id()), [this, &psys, &base](nm::node_info_ptr node)->bool
            {
                if (boost::starts_with(node->name(), "mast_") && node->get_collision())
                {
                    phys_sys::sensor_ptr s = phys_sys::get_sensor(*node->get_collision());
                    if (s->chunks_count() > 0)
                    {
                        this->masts_.push_back(psys->create_static_convex(s, base(node->get_global_pos()), node->get_global_orien()));
                    }
                }

                return true;
            });
        }
#endif
    }


}

} // end of airport
