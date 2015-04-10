#include "stdafx.h"
#include "aircraft_rotors_impl.h"

namespace aircraft
{

    rotors_support_impl::rotors_support_impl(nodes_management::manager_ptr nodes_manager)
        : nodes_manager_(nodes_manager)
    {
        if (auto rotors_group_node = nodes_manager_->find_node("rotorf"))
        {
            rotors_groups_[RG_FRONT] = boost::in_place(rotors_group_node/*, true*/);
            find_rotors(RG_FRONT, rotors_group_node);
        }
        if (auto rotors_group_node = nodes_manager_->find_node("rotorl"))
        {
            rotors_groups_[RG_REAR_LEFT] = boost::in_place(rotors_group_node/*, false*/);
            find_rotors(RG_REAR_LEFT, rotors_group_node);
        }
        if (auto rotors_group_node = nodes_manager_->find_node("rotorr"))
        {
            rotors_groups_[RG_REAR_RIGHT] = boost::in_place(rotors_group_node/*, false*/);
            find_rotors(RG_REAR_RIGHT, rotors_group_node);
        }
    }

    void rotors_support_impl::visit_groups(std::function<void(rotors_group_t &)> out)
    {
        for (size_t i = 0; i < util::array_size(rotors_groups_); ++i)
        {
            if (rotors_groups_[i])
            {
                out(*rotors_groups_[i]);
            }
        }
    }

    void rotors_support_impl::visit_rotors(std::function<void(rotors_group_t const&)> out)
    {
        for (size_t i = 0; i < util::array_size(rotors_groups_); ++i)
        {
            if (rotors_groups_[i])
            {
                //for (auto it= rotors_groups_[i]->shassis.begin(); it != rotors_groups_[i]->shassis.end(); ++it)
                    out(*rotors_groups_[i]/*, *it*/);
            }
        }
    }

    void rotors_support_impl::set_malfunction(rotors_group group, bool val)
    {
        if (rotors_groups_[group])
            rotors_groups_[group]->malfunction = val;
    }

    void rotors_support_impl::freeze()
    {
        for (size_t i = 0; i < util::array_size(rotors_groups_); ++i)
        {
            if (rotors_groups_[i])
            {
                rotors_groups_[i]->freeze();
            }
        }
    }


    void rotors_support_impl::find_rotors( rotors_group i, nm::node_info_ptr group_node )
    {

        auto it = nodes_manager_->get_node_tree_iterator(group_node->node_id());

        nm::visit_sub_tree(it, [this, i](nm::node_info_ptr node)->bool
        {
            std::string name = node->name();
            if (boost::starts_with(node->name(), "rotor"))
            {
                //nm::node_info_ptr wheel_node;
                //nm::visit_sub_tree(this->nodes_manager_->get_node_tree_iterator(node->node_id()), [&wheel_node](nm::node_info_ptr n)->bool
                //{
                //    if (boost::starts_with(n->name(), "wheel"))
                //    {
                //        wheel_node = n;
                //        return false;
                //    }
                //    return true;
                //});

                //if (wheel_node)
                //{
                //    FIXME ("We don't have collision volumes")

                //    if (auto collision = wheel_node->get_collision())
                //    {
                //        cg::rectangle_3 bound = model_structure::bounding(*collision);
                //        double radius = 0.75 * (bound.size().y / 2.);

                //        this->rotors_groups_[i]->add_chassis(shassis_t(node, wheel_node, radius));
                //    }
                //    else
                //    {
                //        this->rotors_groups_[i]->add_chassis(shassis_t(node, wheel_node,  0.75 * node->get_bound().radius));
                //    }

                //}

                this->rotors_groups_[i]->rotor_node  =  node;
            }

            return true;
        });



    }


}
