#include "dubin_route_ctrl.h"

namespace dubin_route
{

object_info_ptr ctrl::create(kernel::object_create_t const& oc, dict_copt dict)
{
    if (dict)
        return object_info_ptr(new ctrl(oc, dict));

    //auto chart_sys = dynamic_cast<kernel::ext_ctrl_sys *>(oc.sys);
    //auto polylin = std::move(polyline_chart_t::create(chart_sys->doc()->default_chart()));

    //if (!polylin)
    //    return object_info_ptr();

    //return object_info_ptr(new ctrl(oc, std::move(app::to_geo_points_2(polylin->get_points()))));
    std::vector<geo_point_2> p;
    p.push_back(geo_point_2(0,0));
    return object_info_ptr(new ctrl(oc, std::move(p)));
}

AUTO_REG_NAME(dubin_route_ext_ctrl, ctrl::create);


ctrl::ctrl(kernel::object_create_t const& oc, dict_copt dict)
    : view                    (oc, dict)
    , ani_                    (find_first_object<ani_object::info_ptr>(collection_))
    , extra_route_chart_      (extra_route_chart(this))
{
    init();
    reset_points();

    // visible(false) ;
}

ctrl::ctrl(kernel::object_create_t const& oc, std::vector<cg::geo_point_2> && points)
    : view                    (oc, points)
    , ani_                    (find_first_object<ani_object::info_ptr>(collection_))
    , extra_route_chart_      (extra_route_chart(this))
{
    init();
    check_segments();
    points_changed();
    reset_points();

    //visible(false) ;
}

void   ctrl::add_point( cg::geo_point_3 const& p )
{
      size_t idx = points_.size();
      point_added_signal_(idx,ani::point_pos(ani::LAYER_GROUND,p));
}

void   ctrl::set_speed( double  speed )
{
      set(msg::settings_msg_t(speed));
}

void ctrl::init()
{
    conn_point_changed_ = subscribe_point_changed(boost::bind(&ctrl::point_dragged, this, _1, _2));
    conn_point_added_   = subscribe_point_added(boost::bind(&ctrl::point_added, this, _1, _2));
    conn_point_removed_ = subscribe_point_removed(boost::bind(&ctrl::point_removed, this, _1));
}

void ctrl::point_dragged(size_t idx, ani::point_pos const& new_pos)
{
    extra_route_chart_->point_dragged(idx, new_pos);
    points_changed();
    reset_points();
}

void ctrl::point_added(size_t idx, ani::point_pos const& new_pos)
{
    //extra_route_chart_->point_added(idx, new_pos);
    //points_changed();
    //reset_points();

    set(msg::add_point_msg(idx,new_pos));
}

void ctrl::point_removed(size_t idx)
{
    extra_route_chart_->point_removed(idx);
    points_changed();
    reset_points();
}

std::vector<ani::point_pos> ctrl::do_check_segment(ani::point_pos const& p, ani::point_pos const& q)
{
    if (ani_)
    {
        ani::navigation_ptr navi = ani_->navigation_info();
        if (auto s = navi->find_point(p))
        {
            if (auto t = navi->find_point(q))
            {                      
                ani::path_type path(std::move(navi->find_shortest_path(*s, *t)));
                if (!path.empty())
                {
                    std::vector<ani::point_pos> free_points;
                    free_points.resize(path.size() - 2);
                    for (size_t i = 0; i < path.size() - 2; ++i)
                        free_points[i] = navi->get_point_info(path[i+1]).pos;

                    return std::vector<ani::point_pos>(std::move(free_points));
                }
            }
        }
    }

    return std::vector<ani::point_pos>();
}

void ctrl::settings_edited()
{
}

void ctrl::reset_points()
{
    std::vector<ani::point_pos> points = std::move(all_points());

    //polyline_chart_t::set_points(to_chart_points(points));
    //for (size_t i = 0; i < points.size(); ++i)
    //    if (!is_anchor(i))
    //        polyline_chart_t::set_point_image(i, ":/images/track-point.bmp");
}

} // dubin_route

