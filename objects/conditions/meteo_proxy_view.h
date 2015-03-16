#pragma once

// #include "common/cloud_zone.h"
#include "common/meteo_proxy.h"
//#include "common/meteo_zone.h"
#include "meteo/manager.h"

namespace meteo_proxy
{
using namespace meteo;

//! представление метеопрокси
struct view
    : base_view_presentation                        //! базовый вид
    , obj_data_holder<wrap_settings<settings_t>>    //! сериализуемые данные из упражнения
    , info                                          //! интерфейс получения информации
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view(kernel::object_create_t const& oc, dict_copt dict);

public:
    ~view() ;

protected:
    void update(double time) override;

    // meteo_proxy::info
protected:
    meteo::meteo_proxy_ptr get_isa_proxy    () const override;
    meteo::meteo_proxy_ptr get_general_proxy() const override;
    meteo::manager_ptr     manager          () const override;

private:
    //void add_zone  (meteo_zone::info_ptr met_zone, size_t obj_id);
    //void add_zone  (cloud_zone::info_ptr cld_zone, size_t obj_id);

    void remove_met_zone(size_t obj_id);
    void remove_cld_zone(size_t obj_id);

    void remove_met_zones();
    void remove_cld_zones();

private:
    void on_path(msg::grib_path_msg const&);

protected:
    void set_grib_path(string const& path);
    virtual void on_grib_path_changed();

protected:
    void on_object_created   (object_info_ptr object);
    void on_object_destroying(object_info_ptr object);

    virtual void on_met_zone_layer_changed(size_t obj_id, size_t layer_num);
    virtual void on_met_zone_geom_changed (size_t obj_id);

    virtual void on_cld_zone_settings_changed(size_t obj_id);

    double       scalar_read(cg::geo_point_3 const&, time_t const&);
    cg::point_2f vector_read(cg::geo_point_3 const&, time_t const&);

private:
    void init_meteo_manager();

protected:
    meteo_proxy_ptr   isa_proxy_;
    manager_ptr       mgr_;
    meteo_proxy_ptr   mgr_proxy_;
    meteo_cursor_ptr  mgr_cursor_; // used only inside meteo_proxy object

private:
    //ph_map<binary::size_type, meteo_zone::info_ptr>::map_t                                      met_zones_;
    //ph_map<binary::size_type, std::array<optional<contour_id>, meteo_zone::num_layers>>::map_t  mgr_met_zns_;

    //ph_map<binary::size_type, cloud_zone::info_ptr>::map_t  cld_zones_;
    //ph_map<binary::size_type, contour_id_set_t>::map_t      mgr_cld_zns_;

    ph_map<binary::size_type, std::unique_ptr<scoped_connection> >::map_t  layer_changed_conns_;
    ph_map<binary::size_type, std::unique_ptr<scoped_connection> >::map_t  geom_changed_conns_;

protected:
    double  last_time_;
};

} // namespace meteo_proxy

