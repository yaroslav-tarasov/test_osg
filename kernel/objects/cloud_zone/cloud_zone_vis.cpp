#include "cloud_zone_vis.h"

#include "common/cloud_zone.h"

namespace cloud_zone
{

object_info_ptr visual::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new visual(oc, dict));
}

AUTO_REG_NAME(cloud_zone_visual, visual::create);

visual::visual(kernel::object_create_t const& oc, dict_copt dict)
    : view(oc, *dict)

    , airports_manager_(find_first_object<airports_manager::info_ptr>(collection_))
    , sys_             (dynamic_cast<visual_system *>(oc.sys))
{
    // This subscription is to avoid update before all airports are loaded
    conn_holder() << dynamic_cast<system_session*>(oc.sys)->subscribe_session_loaded(boost::bind(&visual::on_settings_changed, this));
}

visual::~visual()
{
    if (PK_NONE != settings_.kind)
        sys_->scene()->getEnvWeather()->RemoveLocalBank(object_id()) ;
}

void visual::update( double time )
{
    view::update(time) ;
    // TODO : move on update
}

void visual::on_settings_changed()
{
    airport::info_ptr air = airports_manager_->find_closest_airport(settings_.points[0]);
    if (!air || !airport::vis_info_ptr(air)->is_visible())
        return ;

    if (cg::distance2d(air->pos(), settings_.points.front()) > 15000.)
        return ;

    if (PK_NONE == settings_.kind)
    {
        sys_->scene()->getEnvWeather()->RemoveLocalBank(object_id()) ;
        return ;
    }

    av::environment_weather::local_bank_data bd;

    geo_base_2 base = dynamic_cast<visual_system_props*>(sys_)->vis_props().base_point ;
    cg::geo_rect_2 obb = cg::bounding(util::raw_ptr(settings_.points), settings_.points.size()) ;

    bd.pos     = cg::point_3f(base(obb.center()), settings_.heights.lo()) ;
    bd.ellipse = 0.5 * cg::point_2f(cg::distance2d(cg::geo_point_2(0., obb.lon.lo()), cg::geo_point_2(0., obb.lon.hi())),
                                    cg::distance2d(cg::geo_point_2(obb.lat.lo(), 0.), cg::geo_point_2(obb.lat.hi(), 0.)));
    bd.heading         = 0.f ;
    bd.type            = av::weather_params::precipitation_type(settings_.kind - 1) ;
    bd.intensity       = 1. ;
    bd.central_portion = 0.6f ;

    sys_->scene()->getEnvWeather()->UpdateLocalBank(object_id(), bd);
}

} // end of cloud_zone



