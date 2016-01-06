#include "stdafx.h"
#include "precompiled_objects.h"

#include "meteo_proxy_view.h"

namespace meteo_proxy
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(meteo_proxy_view, view::create);

view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base(dict)
    , isa_proxy_(create_isa_proxy())
    , last_time_(0.)
{
    init_meteo_manager();
    on_grib_path_changed();

    msg_disp().
        add<msg::grib_path_msg>(boost::bind(&view::on_path, this, _1));
}

view::~view()
{
    remove_met_zones();
    remove_cld_zones();
}

void view::update(double time)
{
    last_time_ = time;
}

meteo::meteo_proxy_ptr view::get_isa_proxy () const
{
    return isa_proxy_;
}

meteo::meteo_proxy_ptr view::get_general_proxy() const
{
    return mgr_proxy_;
}

meteo::manager_ptr view::manager() const
{
    return mgr_;
}

//void view::add_zone(meteo_zone::info_ptr met_zone, size_t obj_id)
//{
//    met_zones_          [obj_id] = met_zone;
//    layer_changed_conns_[obj_id].reset(new scoped_connection(
//        met_zone->subscribe_layer_changed(boost::bind(&view::on_met_zone_layer_changed, this, obj_id, _1))));
//    _geomchanged_conns_ [obj_id].reset(new scoped_connection(
//        met_zone->subscribe_geometry_changed(boost::bind(&view::on_met_zone__geomchanged, this, obj_id))));
//
//    for (size_t i = 0; i < met_zone->get_layers().size(); ++i)
//        if (met_zone->get_layers()[i])
//            mgr_met_zns_[obj_id][i] = mgr_->add(met_zone->get_points(), met_zone->get_layers()[i]);
//}
//
//void view::add_zone(cloud_zone::info_ptr cld_zone, size_t obj_id)
//{
//    cld_zones_[obj_id] = cld_zone;
//    layer_changed_conns_[obj_id].reset(new scoped_connection(
//        cld_zone->subscribe_settings_changed(boost::bind(&view::on_cld_zone_settings_changed, this, obj_id))));
//
//    mgr_cld_zns_[obj_id].insert(mgr_->add(cld_zone->settings().points, cld_zone));
//}

void view::remove_met_zone(size_t obj_id)
{
    //auto const zbeg = mgr_met_zns_[obj_id].begin();
    //auto const zend = mgr_met_zns_[obj_id].end();
    //for (auto mz = zbeg; mz != zend; ++mz)
    //    if (*mz)
    //        mgr_->remove(**mz);

    //layer_changed_conns_.erase(obj_id);
    //_geomchanged_conns_ .erase(obj_id);
    //met_zones_          .erase(obj_id);
    //mgr_met_zns_        .erase(obj_id);
}

void view::remove_cld_zone(size_t obj_id)
{
    //auto const zbeg = mgr_cld_zns_[obj_id].begin();
    //auto const zend = mgr_cld_zns_[obj_id].end();
    //for (auto mz = zbeg; mz != zend; ++mz)
    //    mgr_->remove(*mz);

    //cld_zones_  .erase(obj_id);
    //mgr_cld_zns_.erase(obj_id);
}

void view::remove_met_zones()
{
    //for (auto it = mgr_met_zns_.begin(), end = mgr_met_zns_.end(); it != end; ++it)
    //    for (auto mit = it->second.begin(), mend = it->second.end(); mit != mend; ++mit)
    //        if (*mit)
    //            mgr_->remove(**mit);

    //layer_changed_conns_.clear();
    //_geomchanged_conns_ .clear();
    //met_zones_          .clear();
    //mgr_met_zns_        .clear();
}

void view::remove_cld_zones()
{
    //for (auto it = mgr_cld_zns_.begin(), end = mgr_cld_zns_.end(); it != end; ++it)
    //    for (auto cit = it->second.begin(), cend = it->second.end(); cit != cend; ++cit)
    //        mgr_->remove(*cit);

    //cld_zones_  .clear();
    //mgr_cld_zns_.clear();
}

void view::on_path(msg::grib_path_msg const& m)
{
    settings_.grib_path = m.path;
    on_grib_path_changed();
}

void view::set_grib_path(string const& path)
{
    set(msg::grib_path_msg(path));
}

void view::on_grib_path_changed()
{
    if (!settings_.grib_path.empty())
        mgr_->load_grib((boost::filesystem::path(cfg().path.data) / settings_.grib_path).string(), "");
    else
        mgr_->clear_grib();
}

void view::on_object_created(object_info_ptr object)
{
    base_view_presentation::on_object_created(object);

    //if (meteo_zone::info_ptr met_zone = object)
    //    add_zone(met_zone, object->object_id());
    //else if (cloud_zone::info_ptr cld_zone = object)
    //    add_zone(cld_zone, object->object_id());
}

void view::on_object_destroying(object_info_ptr object)
{
    base_view_presentation::on_object_destroying(object) ;

    //if (meteo_zone::info_ptr met_zone = object)
    //    remove_met_zone(object->object_id());
    //else if (cloud_zone::info_ptr cld_zone = object)
    //    remove_cld_zone(object->object_id());
}

void view::on_met_zone_layer_changed(size_t obj_id, size_t /*layer_num*/)
{
    //Assert(met_zones_.find(obj_id) != met_zones_.end());

    //meteo_zone::info_ptr mzi = met_zones_[obj_id];

    //remove_met_zone(obj_id);
    //add_zone(mzi, obj_id);

    //meteo_zone_changed_signal_(mzi);
}

void view::on_met_zone_geom_changed(size_t obj_id)
{
    //Assert(met_zones_.find(obj_id) != met_zones_.end());

    //meteo_zone::info_ptr mzi = met_zones_[obj_id];

    //remove_met_zone(obj_id);
    //add_zone(mzi, obj_id);

    //meteo_zone_changed_signal_(mzi);
}

void view::on_cld_zone_settings_changed(size_t obj_id)
{
    //Assert(cld_zones_.find(obj_id) != cld_zones_.end());

    //cloud_zone::info_ptr cldi = cld_zones_[obj_id];

    //remove_cld_zone(obj_id);
    //add_zone(cldi, obj_id);

    //cloud_zone_changed_signal_(cldi);
}

double view::scalar_read(cg::geo_point_3 const& pos, time_t const& forec_time)
{
    mgr_cursor_->move_to(pos, forec_time);
    return mgr_cursor_->air_temperature();
}

cg::point_2f view::vector_read(cg::geo_point_3 const& pos, time_t const& forec_time)
{
    mgr_cursor_->move_to(pos, forec_time);
    return mgr_cursor_->wind();
}

void view::init_meteo_manager()
{
    cg::range_2 const& lat = settings_.lat_bounds;
    cg::range_2 const& lon = settings_.lon_bounds;

    double const num_points = 4096.;
    if (lat.empty() || lon.empty())
        return;
    else
    {
        double step = 0.015;
        int lat_pnts = int(lat.size() / step);
        int lon_pnts = int(lon.size() / step);
        if (lat_pnts * lon_pnts > num_points)
        {
            double dec = cg::sqrt((lat_pnts * lon_pnts) / num_points);
            lat_pnts = lat_pnts / dec + 1;
            lon_pnts = lon_pnts / dec + 1;
            step = lat.size() / lat_pnts;
        }

        if (!mgr_)
        {
            mgr_        = create_manager(cg::geo_point_2(lat.lo(), lon.lo()), cg::geo_point_2(step, step), cg::point_2i(lat_pnts, lon_pnts));
            mgr_proxy_  = mgr_->get_proxy();
            mgr_cursor_ = mgr_proxy_->create_cursor();
        }
        else
            mgr_->resize(cg::geo_point_2(lat.lo(), lon.lo()), cg::geo_point_2(step, step), cg::point_2i(lat_pnts, lon_pnts));
    }
}

} // namespace meteo_proxy

