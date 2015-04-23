#include "stdafx.h"
#include "precompiled_objects.h"

#include "dubin_route_view.h"
#include "reflection/proc/binary.h"


namespace dubin_route
{
namespace 
{
inline std::vector<anchor_point_t> make_anchor_points(std::vector<geo_point_2> const& points)
{
    std::vector<anchor_point_t> anchor_points;

    for (auto pp = points.begin(); pp != points.end(); ++pp)
        anchor_points.push_back(anchor_point_t(ani::ground_point(*pp)));

    return anchor_points;
}
} // 'anonymous'

view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
    , extra_route_chart_    (extra_route_chart(this))
{
    points_changed();
    msg_disp()
        .add<msg::add_point_msg   >(boost::bind(&view::on_point_added   , this, _1))
        .add<msg::settings_msg_t>  (boost::bind(&view::on_settings, this, _1))
        ;
}

view::view(kernel::object_create_t const& oc, std::vector<geo_point_2> const& points)
    : base_view_presentation(oc)
    , obj_data_base         (settings_t(),make_anchor_points(points))
    , extra_route_chart_    (extra_route_chart(this))
{
    points_changed();
    msg_disp()
        .add<msg::add_point_msg   >(boost::bind(&view::on_point_added   , this, _1))
        .add<msg::settings_msg_t>  (boost::bind(&view::on_settings, this, _1))
        ;
}

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, *dict));
}

double view::length() const
{       
    return length_; 
}

geo_point_2 view::interpolate( double t ) const
{   
    size_t i;
    for (i = 0; i < dist_.size() - 1 && dist_[i+1] < t; ++i);

    if (i < dist_.size())
    {
        double param = (t - dist_[i])/(dist_[i+1] - dist_[i]) ;
        return cg::blend((geo_point_2)points_[i], (geo_point_2)points_[i+1], param);
    }

    return points_.back();
}

double view::interpolate_speed( double t ) const
{   
    size_t i;
    for (i = 0; i < dist_.size() - 1 && dist_[i+1] < t; ++i);

    if (i < dist_.size())
    {
        double param = (t - dist_[i])/(dist_[i+1] - dist_[i]) ;
        return cg::blend(speed_[i], speed_[i+1], param);
    }

    return 0.;
}

double view::closest( geo_point_3 const& p ) const
{
    optional<double> mindist;
    double mint = 0;

    for(size_t i = 0; i < points_.size() - 1; ++i)
    {
        cg::geo_segment_2 seg(points_[i], points_[i+1], true);
        geo_point_2 closest = seg.closest_point(p);
        double dist = cg::distance(closest, p);

        if (!mindist || dist < mindist)
        {
            mindist = dist;
            mint = cg::clamp(0., 1., dist_[i], dist_[i+1])(seg(closest)) ;
        }
    }

    return mint;
}

//void view::on_msg( short /*id*/, const void * /*data*/, size_t /*size*/ )
//{
//}

void view::point_dragged(size_t idx, ani::point_pos const& new_pos)
{
    extra_route_chart_->point_dragged(idx, new_pos);
    points_changed();
    reset_points();
}

void view::on_point_added(msg::add_point_msg const& pnt)
{
     point_added(pnt.idx,ani::point_pos(0,pnt.pos));
     LOG_ODS_MSG("on_point_added -------\n" 
                << "pnt.idx=" << pnt.idx 
                << " pnt.pos.x= " << pnt.pos.lat 
                << " pnt.pos.y= " << pnt.pos.lon 
                << " anchor_points().size()= " << anchor_points().size()
                << "\n");
}

void view::on_settings(msg::settings_msg_t const& settings)
{
      settings_ = settings;
}

void view::point_added(size_t idx, ani::point_pos const& new_pos)
{

    extra_route_chart_->point_added(idx, new_pos);
    
    points_changed();
    reset_points();
}

void view::point_removed(size_t idx)
{
    extra_route_chart_->point_removed(idx);
    points_changed();
    reset_points();
}

void view::points_changed()
{
    points_ = all_points();
	
	speed_.clear();

    for (size_t i = 0; i < points_.size(); ++i)
    {
        size_t anchor =get_anchor(i);
        if (anchor_points()[anchor].speed > 0)
            speed_.push_back(anchor_points()[anchor].speed);
        else
            speed_.push_back(settings_.speed);
    }
	
	if(speed_.size()>1)
		speed_.back() = 0;

    length_ = 0;

    dist_.resize(points_.size());
    if(points_.size()>0)
    {
        dist_[0] = 0;

        FIXME(Не нуачо классный код (unsigned))
        for (size_t i = 0; i < points_.size() - 1; ++i)
        {
            double dst = cg::distance2d(points_[i], points_[i+1]);
            length_ += dst;

            dist_[i + 1] = length_;
        }
    }

}



AUTO_REG_NAME(dubin_route_view, view::create);

} // namespace fpl
