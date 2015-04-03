#pragma once

#include "common/airports_manager.h"

namespace airports_manager
{
struct view
    : base_view_presentation
    , info
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view(kernel::object_create_t const& oc, dict_copt dict);

    // base_view
protected:
    void on_object_created(object_info_ptr object) override;
    void on_object_destroying(object_info_ptr object) override;

    // info
protected:
    airport::info_ptr find_closest_airport(geo_point_2 const& pos) const override;

private:
    std::set<airport::info_ptr> airports_;

};

}
