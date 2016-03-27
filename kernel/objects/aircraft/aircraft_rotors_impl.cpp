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

		if (auto rotors_group_node = nodes_manager_->find_node("rotor_l"))
		{
			rotors_groups_[RG_REAR_LEFT] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_LEFT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotor_r"))
		{
			rotors_groups_[RG_REAR_RIGHT] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_RIGHT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotorl1"))
		{
			rotors_groups_[RG_REAR_LEFT] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_LEFT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotorr1"))
		{
			rotors_groups_[RG_REAR_RIGHT] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_RIGHT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotorl2"))
		{
			rotors_groups_[RG_REAR_LEFT2] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_LEFT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotorr2"))
		{
			rotors_groups_[RG_REAR_RIGHT2] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_RIGHT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotorl3"))
		{
			rotors_groups_[RG_REAR_LEFT3] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_LEFT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("rotorr3"))
		{
			rotors_groups_[RG_REAR_RIGHT3] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_REAR_RIGHT, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("mainrotor"))
		{
			rotors_groups_[RG_MAIN] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_MAIN_1, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("mainrotor1"))
		{
			rotors_groups_[RG_MAIN_1] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_MAIN_1, rotors_group_node);
		}

		if (auto rotors_group_node = nodes_manager_->find_node("mainrotor2"))
		{
			rotors_groups_[RG_MAIN_2] = boost::in_place(rotors_group_node/*, false*/);
			find_rotors(RG_MAIN_2, rotors_group_node);
		}

    }

    void rotors_support_impl::visit_groups(std::function<void(rotors_group_t &,size_t&)> out)
    {
        for (size_t i = 0; i < util::array_size(rotors_groups_); ++i)
        {
            if (rotors_groups_[i])
            {
                out(*rotors_groups_[i],i);
            }
        }
    }

    void rotors_support_impl::visit_rotors(std::function<void(rotors_group_t const&,size_t&)> out)
    {
        for (size_t i = 0; i < util::array_size(rotors_groups_); ++i)
        {
            if (rotors_groups_[i])
            {
                //for (auto it= rotors_groups_[i]->shassis.begin(); it != rotors_groups_[i]->shassis.end(); ++it)
                    out(*rotors_groups_[i],i/*, *it*/);
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
            const std::string& name = node->name();
            const rotors_group t = static_cast<rotors_group>(i); 

            LogInfo("rotors_support_impl::find_rotors: " << name << "group #" << i );
           
            if (boost::starts_with(node->name(), "rotordyn"))
            {
                this->rotors_groups_[i]->dyn_rotor_node  =  node;
            }
            else
            if (boost::starts_with(node->name(), "rotorstatic"))
            {
                this->rotors_groups_[i]->rotor_node  =  node;
            }
            else
            if (boost::starts_with(node->name(), "rotorsagged"))
            {
                this->rotors_groups_[i]->sag_rotor_node  =  node;
            }

            return true;
        });



    }


}
