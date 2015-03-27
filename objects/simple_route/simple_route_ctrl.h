#pragma once

#include "simple_route_view.h"

#include "objects/ani.h"
#include "impl/extra_route/extra_route_chart.h"

namespace simple_route
{


struct ctrl
    : view
    , control
{
   typedef extra_route::chart<anchor_point_t> extra_route_chart;

   static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

    private:
        ctrl(kernel::object_create_t const& oc, dict_copt dict);
        ctrl(kernel::object_create_t const& oc, std::vector<cg::geo_point_2> && points);

    // control
public:
    void add_point(cg::geo_point_3 const& p) override;
    void set_speed( double  speed ) override;
    // own
private:
    void init           ();

    void point_dragged  (size_t pnt_id, ani::point_pos const& new_pos);
    void point_added    (size_t pnt_id, ani::point_pos const& new_pos);
    void point_removed  (size_t pnt_id) ;

    std::vector<ani::point_pos> do_check_segment(ani::point_pos const& p, ani::point_pos const& q);
    void settings_edited();
    virtual void reset_points() override;

private:
    ani_object::info_ptr ani_;
    optional<extra_route_chart> extra_route_chart_;

private:
    scoped_connection  conn_point_changed_;
    scoped_connection  conn_point_added_;
    scoped_connection  conn_point_removed_;

public:
    DECLARE_EVENT(point_added  , (size_t, ani::point_pos const&));
    DECLARE_EVENT(point_removed, (size_t));
    DECLARE_EVENT(point_changed, (size_t, ani::point_pos const&));
};

} 