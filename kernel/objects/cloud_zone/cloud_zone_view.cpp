#include "precompiled_objects.h"
#include "cloud_zone_view.h"

namespace cloud_zone
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    if (dict)
        return object_info_ptr(new view(oc, *dict));

    return object_info_ptr();
}

AUTO_REG_NAME(cloud_zone_view, view::create);

view::view( kernel::object_create_t const& oc, dict_t const& dict )
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{
    make_grid();
    msg_disp()
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1));
}

view::view( kernel::object_create_t const& oc, points_t const& points )
    : base_view_presentation(oc)
    , obj_data_base         (points)
{
    make_grid();
    msg_disp()
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1));
}

cg::range_2 const& view::height() const
{
    return settings_.heights;
}

cg::range_2ui const& view::time() const
{
    return settings_.time;
}

unsigned view::kind() const
{
    return settings_.kind;
}

double view::intensity() const
{
    return settings_.inten;
}

settings_t const& view::settings() const
{
    return settings_;
}

bool view::is_inside(geo_point_3 const& pos) const
{
    if (!grid_)
        return false;

    cg::geo_base_2 base(settings_.points.front()) ;
    return grid_->contains_point(base(pos));
}

void view::on_settings(msg::settings_msg const& m)
{
    settings_ = m.settings;
    on_settings_changed();
    settings_changed_signal_();
}

void view::make_grid()
{
    Assert(settings_.points.size() > 2);

    std::vector<cg::point_2> points ;
    cg::geo_base_2 base(settings_.points.front()) ;
    for (auto it = settings_.points.begin(), end = settings_.points.end(); it != end; ++it)
        points.emplace_back(base(*it)) ;

    grid_ = boost::in_place();
    grid_->add_contour(points.begin(), points.end());
    grid_->process();
}

}

