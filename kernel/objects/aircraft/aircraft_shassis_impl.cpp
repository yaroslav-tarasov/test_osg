#include "stdafx.h"
#include "aircraft_shassis_impl.h"

namespace aircraft
{

    shassis_support_impl::shassis_support_impl(nodes_management::manager_ptr nodes_manager)
        : nodes_manager_(nodes_manager)
    {
        if (auto shassis_group_node = nodes_manager_->find_node("animgroup_shassi_f"))
        {
            shassis_groups_[SG_FRONT] = boost::in_place(shassis_group_node, true);
            find_shassis(SG_FRONT, shassis_group_node);
        }
        if (auto shassis_group_node = nodes_manager_->find_node("animgroup_shassi_r_l"))
        {
            shassis_groups_[SG_REAR_LEFT] = boost::in_place(shassis_group_node, false);
            find_shassis(SG_REAR_LEFT, shassis_group_node);
        }
        if (auto shassis_group_node = nodes_manager_->find_node("animgroup_shassi_r_r"))
        {
            shassis_groups_[SG_REAR_RIGHT] = boost::in_place(shassis_group_node, false);
            find_shassis(SG_REAR_RIGHT, shassis_group_node);
        }
    }

    void shassis_support_impl::visit_groups(std::function<void(shassis_group_t &)> out)
    {
        for (size_t i = 0; i < util::array_size(shassis_groups_); ++i)
        {
            if (shassis_groups_[i])
            {
                out(*shassis_groups_[i]);
            }
        }
    }

    void shassis_support_impl::visit_chassis(std::function<void(shassis_group_t const&, shassis_t&)> out)
    {
        for (size_t i = 0; i < util::array_size(shassis_groups_); ++i)
        {
            if (shassis_groups_[i])
            {
                for (auto it= shassis_groups_[i]->shassis.begin(); it != shassis_groups_[i]->shassis.end(); ++it)
                    out(*shassis_groups_[i], *it);
            }
        }
    }

    void shassis_support_impl::set_malfunction(shasis_group group, bool val)
    {
        if (shassis_groups_[group])
            shassis_groups_[group]->malfunction = val;
    }

    void shassis_support_impl::freeze()
    {
        for (size_t i = 0; i < util::array_size(shassis_groups_); ++i)
        {
            if (shassis_groups_[i])
            {
                shassis_groups_[i]->freeze();
            }
        }
    }


#if 0
    void shassis_support_impl::find_shassis( shasis_group i, nm::node_info_ptr group_node )
    {

        auto it = nodes_manager_->get_node_tree_iterator(group_node->node_id());

        nm::visit_sub_tree(it, [this, i](nm::node_info_ptr node)->bool
        {
            const std::string & name = node->name();
            if (boost::starts_with(node->name(), "shassi_"))
            {
                nm::node_info_ptr wheel_node;
                nm::visit_sub_tree(this->nodes_manager_->get_node_tree_iterator(node->node_id()), [&wheel_node](nm::node_info_ptr n)->bool
                {
                    if (boost::starts_with(n->name(), "wheel_"))
                    {
                        wheel_node = n;
                        return false;
                    }
                    return true;
                });

                if (wheel_node)
                {
                    FIXME ("We don't have collision volumes")

                    if (auto collision = wheel_node->get_collision())
                    {
                        cg::rectangle_3 bound = model_structure::bounding(*collision);
                        double radius = 0.75 * (bound.size().y / 2.);

                        this->shassis_groups_[i]->add_chassis(shassis_t(node, wheel_node, radius));
                    }
                    else
                    {
                        double radius = get_wheel_radius(node);//0.75 * (node->get_bound().size().z / 2.);
                        this->shassis_groups_[i]->add_chassis(shassis_t(node, wheel_node,  radius/*0.75 * node->get_bound().radius*/));
                    }

                }
            }

            return true;
        });



    }
#else
    void shassis_support_impl::find_shassis( shasis_group i, nm::node_info_ptr group_node )
    {

        auto it = nodes_manager_->get_node_tree_iterator(group_node->node_id());

        nm::visit_sub_tree(it, [this, i](nm::node_info_ptr node)->bool
        {
            const std::string & name = node->name();
            if (boost::starts_with(node->name(), "shassi_"))
            {
                std::vector<nm::node_info_ptr> wheel_nodes;
                nm::visit_sub_tree(this->nodes_manager_->get_node_tree_iterator(node->node_id()), [&wheel_nodes](nm::node_info_ptr n)->bool
                {
                    if (boost::starts_with(n->name(), "wheel_"))
                    {
                        wheel_nodes.push_back(n);
                    }
                    return true;
                });

                for (auto it = wheel_nodes.begin(); it != wheel_nodes.end(); ++it )
                {
                    FIXME ("We don't have collision volumes")
                        auto wheel_node = (*it);

                        if (auto collision = wheel_node->get_collision())
                        {
                            cg::rectangle_3 bound = model_structure::bounding(*collision);
                            double radius = 0.75 * (bound.size().y / 2.);

                            this->shassis_groups_[i]->add_chassis(shassis_t(node, wheel_node, radius));
                        }
                        else
                        {
                            double radius = get_wheel_radius(node);//0.75 * (node->get_bound().size().z / 2.);
                            this->shassis_groups_[i]->add_chassis(shassis_t(node, wheel_node,  radius/*0.75 * node->get_bound().radius*/));
                        }

                }
            }

            return true;
        });



    }
#endif


}
