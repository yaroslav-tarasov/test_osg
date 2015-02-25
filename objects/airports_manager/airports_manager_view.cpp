#include "airports_manager_view.h"

namespace airports_manager
{
object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(airports_manager_view, view::create);
AUTO_REG_NAME(airports_manager_chart, base_chart_presentation<view>::create);

view::view( kernel::object_create_t const& oc, dict_copt)
    : base_view_presentation(oc)
{
}

void view::on_object_created(object_info_ptr object)
{
    base_view_presentation::on_object_created(object);

    if (auto air = airport::info_ptr(object))
    {
        airports_.insert(air);
    }
}

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object) ;

    if (auto air = airport::info_ptr(object))
    {
        airports_.erase(air);
    }
}

airport::info_ptr view::find_closest_airport(geo_point_2 const& pos) const
{
    airport::info_ptr closest;
    double closest_dist = std::numeric_limits<double>::max();

    for (auto it = airports_.begin(); it != airports_.end(); ++it)
    {
        double dist = cg::distance2d((*it)->pos(), pos);
        if (!closest || dist < closest_dist)
        {
            closest = *it;
            closest_dist = dist;
        }                                 
    }

    return closest;
}
} // airports_manager
