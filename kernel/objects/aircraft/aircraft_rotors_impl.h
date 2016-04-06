#pragma once

#include "phys/phys_sys.h"
#include "aircraft_common.h"
#include "common/chassis_common.h"
#include "common/rotors_common.h"

namespace aircraft
{
    struct rotors_support_impl
        : rotors_support
    {
        rotors_support_impl(nodes_management::manager_ptr nodes_manager);

    private:
        void visit_groups   (std::function<void(rotors_group_t &,size_t&)> out);
        void visit_rotors   (std::function<void(rotors_group_t const&,size_t&)> out);
        void set_malfunction(rotors_group group, bool val);
        void freeze();

    private:
        void find_rotors( rotors_group i, nm::node_info_ptr group_node );

    private:
        nodes_management::manager_ptr nodes_manager_;
        optional<rotors_group_t>      rotors_groups_[RG_LAST];

    private:
        DECL_LOGGER("rotors_support_impl");

    };

}
