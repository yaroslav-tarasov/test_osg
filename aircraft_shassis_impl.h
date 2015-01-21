#pragma once

#include "phys_sys.h"
#include "aircraft_common.h"

namespace aircraft
{
    struct shassis_support_impl
        : shassis_support
    {
        shassis_support_impl(nodes_management::manager_ptr nodes_manager);

        // shassis_support
    private:
        void visit_groups   (std::function<void(shassis_group_t &)> out);
        void visit_chassis  (std::function<void(shassis_group_t const&, shassis_t&)> out);
        void set_malfunction(shasis_group group, bool val);
        void freeze();

    private:
        void find_shassis( shasis_group i, nm::node_info_ptr group_node );

    private:
        nodes_management::manager_ptr nodes_manager_;
        optional<shassis_group_t>     shassis_groups_[3];
    };

}