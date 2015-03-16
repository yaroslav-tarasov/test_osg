#pragma once

//#include "common/cloud_zone.h"
//#include "common/meteo_zone.h"
#include "meteo/meteo.h"

namespace meteo_proxy
{
//! интерфейс информации о метео
struct info
{
    virtual meteo::meteo_proxy_ptr  get_isa_proxy    () const = 0;
    virtual meteo::meteo_proxy_ptr  get_general_proxy() const = 0;
    virtual meteo::manager_ptr      manager          () const = 0;

    // DECLARE_EVENT(meteo_zone_changed, (meteo_zone::info_ptr));
    // DECLARE_EVENT(cloud_zone_changed, (cloud_zone::info_ptr));

    virtual ~info() { }
};

typedef polymorph_ptr<info>  info_ptr;

//! настройки отображения метео (?)
struct show_settings
{
    show_settings()
        : by_height(0)
        , by_time  (0)
        , show_scalar(false)
        , show_vector(false)
    {}

    double   by_height;
    unsigned by_time;

    bool show_scalar;
    bool show_vector;
};

//! настройки метео - сериализуемые данные из упражнения
struct settings_t
{
    explicit settings_t(std::string data_path = std::string())
        : grib_path (data_path)
        , lat_bounds(40., 70.)
        , lon_bounds(20., 150.)
    { }

    show_settings show;         // параметры отображения

    std::string  grib_path;     // путь к GRIB (???)

    cg::range_2  lat_bounds;    // диапазон по широте
    cg::range_2  lon_bounds;    // диапазон по долготе
};

namespace msg
{
//! сообщения - идентификатор сообщения от метео
enum id
{
    mpm_grib_path = 1
};

//! сообщение от метео
struct grib_path_msg 
    : network::msg_id<mpm_grib_path>
{
    grib_path_msg(string const& path = "")
        : path(path)
    {
    }

    string path;
};

REFL_STRUCT(grib_path_msg)
    REFL_ENTRY(path)
REFL_END   ()

} // msg

REFL_STRUCT(show_settings)
    REFL_NUM  (by_height , 0, 15000, 100)
    REFL_TIME (by_time)
    REFL_ENTRY(show_scalar)
    REFL_ENTRY(show_vector)
REFL_END()

REFL_STRUCT(settings_t)
    REFL_ENTRY   (show)
    REFL_SER     (grib_path)
    REFL_SER     (lat_bounds)
    REFL_SER     (lon_bounds)
REFL_END()

} // namespace meteo_proxy