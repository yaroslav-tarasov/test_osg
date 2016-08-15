#pragma once

#include "cloud_zone_common.h"

#include "geometry/contours/contours_set.h"

namespace cloud_zone
{

// базовый класс для всех представлений облачности (на карте, в моделировании, на визуализации)
struct view
    : base_view_presentation                        // базовый "вид"
    , obj_data_holder<wrap_settings<settings_t>>    // данные
    , info                                          // интерфейс информации об облачности
{
    typedef settings_t::points_t points_t;

    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

protected:
    view( kernel::object_create_t const& oc, dict_t const& dict );
    view( kernel::object_create_t const& oc, points_t const& points );

    // own
protected:
    cg::range_2   const&  height    () const;
    cg::range_2ui const&  time      () const;
    unsigned              kind      () const;
    double                intensity () const;

    // info
protected:
    settings_t const& settings () const override;
    bool              is_inside(geo_point_3 const& pos) const override;

private:
    void on_settings(msg::settings_msg const& msg);

private:
    void make_grid();

protected:
    virtual void on_settings_changed() { }

protected:
    optional<cg::contours::gen::nested_contours_set<>> grid_;
};

} // end of cloud_zone
