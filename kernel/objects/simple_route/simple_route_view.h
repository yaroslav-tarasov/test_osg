#pragma once

#include "common/simple_route.h"
#include "impl/extra_route/extra_route_view.h"
#include "impl/extra_route/extra_route_chart.h"
#include "simple_route_msg.h"

namespace extra_route
{
    template<>
    inline ani::point_pos anchor_point_get_pos< ::simple_route::anchor_point_t>(::simple_route::anchor_point_t const& pnt)
    {
        return ani::point_pos(pnt.pos);
    }

    template<>
    inline void anchor_point_set_pos< ::simple_route::anchor_point_t>(::simple_route::anchor_point_t & pnt, ani::point_pos const& pos)
    {
        pnt.pos = pos ;
    }
}

namespace simple_route
{
    struct route_data
        : extra_route::view<anchor_point_t>
    {
     //protected:// TYV обойдетесь 
        typedef extra_route::view<anchor_point_t>  route_base_t;
        typedef extra_route::chart<anchor_point_t> extra_route_chart;

        route_data()
        {
        }

        route_data(settings_t const& settings,anchor_points_t const& anchor_points)
            : settings_(settings)
            , route_base_t(anchor_points)
        {
        }

    protected:
        settings_t  settings_;

        REFL_INNER(route_data)
            REFL_ENTRY(settings_)
            REFL_CHAIN(route_base_t)
        REFL_END()
    };

    //! представление простого маршрута
    struct view
        : base_view_presentation        
        , info                          
        , obj_data_holder<route_data>   
    {
        typedef extra_route::view<anchor_point_t> extra_route_view;

        view(kernel::object_create_t const& oc, dict_copt dict);
        view(kernel::object_create_t const& oc, std::vector<geo_point_2> const& points);

        static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

        // info
    protected:
        double length() const;
        geo_point_2 interpolate( double t ) const;
        double interpolate_speed( double t ) const;
        double closest( geo_point_3 const& p ) const;

        // app::base_presentation
    protected:
        //void on_msg(short id, const void * data, size_t size);
    private:
        virtual void reset_points(){};
        void point_dragged  (size_t pnt_id, ani::point_pos const& new_pos);
        void point_added    (size_t pnt_id, ani::point_pos const& new_pos);
        void point_removed  (size_t pnt_id) ;

        void on_point_added    (msg::add_point_msg const& msg);
        void on_settings       (msg::settings_msg_t const& settings);

    protected:
        void set_point(size_t pnt_id, cg::geo_point_3 const& new_pos);
        void add_point(size_t pnt_id, cg::geo_point_3 const& new_pos);
        void remove_point(size_t pnt_id);

    protected:
        void points_changed();

    protected:
        std::vector<double>         dist_;
        std::vector<ani::point_pos> points_;
        std::vector<double>         speed_;
        double                      length_;
    
    private:
        optional<extra_route_chart> extra_route_chart_;

    };

}
