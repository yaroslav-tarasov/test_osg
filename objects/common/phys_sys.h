#pragma once

#include "phys/phys_sys.h"

namespace phys
{
    struct control
    {
        virtual ~control() {}

        virtual system_ptr        get_system(size_t zone) = 0;

        virtual optional<size_t>  get_zone(geo_point_3 const & pos) const = 0;
        virtual optional<size_t> get_zone(std::string const& airport) const = 0;
        virtual geo_base_3 const& get_base(size_t zone) const = 0;
        virtual string zone_name(size_t id) const = 0;

        DECLARE_EVENT(zone_created, (size_t));
        DECLARE_EVENT(zone_destroyed, (size_t));
    };



    typedef polymorph_ptr<control> control_ptr;
}