#include "stdafx.h"
#include "dubins.h"
#include "trajectory.h"

namespace fms
{

struct trajectory_impl;

trajectory_ptr trajectory::create(const decart_position& begin_pos,decart_position const& end_pos,double radius, double step)
{
    return boost::make_shared<trajectory_impl>(begin_pos,end_pos,radius,step);
}

trajectory_ptr trajectory::create(const traj_data& data)
{
    return boost::make_shared<trajectory_impl>(data);
}

trajectory_ptr trajectory::create(const keypoints_t& kpts,curses_t const& crs,speed_t const& vel)
{
    return boost::make_shared<trajectory_impl>(kpts,crs,vel);
}


typedef polymorph_ptr<trajectory_impl> trajectory_impl_ptr;

struct trajectory_impl : trajectory
{   
    trajectory_impl(const decart_position& begin_pos,decart_position const& end_pos,double radius, double step = 1)
        : curr_pos_(0)
    {
        double q0[] = { begin_pos.pos.x,begin_pos.pos.y,to_dubin( begin_pos.orien) };
        double q1[] = { end_pos.pos.x,end_pos.pos.y,to_dubin( end_pos.orien)};
        keypoints_t         kpts;
        curses_t             crs;
        DubinsPath          path;
        dubins_init( q0, q1, radius, &path);
        dubins_path_sample_many( &path,std::bind(fill,std::ref(kpts),std::ref(crs),sp::_1,sp::_2,sp::_3), step, nullptr);
        kp_seg_.push_back(kpts);
        curs_seg_.push_back(crs);
    }
    
    trajectory_impl(const keypoints_t& kpts ,curses_t const& crs,speed_t const& vel )   
        : curr_pos_(0)
    {

        kp_seg_.push_back(kpts);
        curs_seg_.push_back(crs);
        (*speed_seg_).push_back(vel);
    }

    trajectory_impl(const trajectory_impl_ptr& other)
    {
        append(other);
    }

    trajectory_impl(const traj_data& other)
    {
        append(other);
    }

    void append(const trajectory_ptr o)     override
    {
        trajectory_impl_ptr other = trajectory_impl_ptr(o);

        const auto length_ = length();
        
        for(auto it = other->kp_seg_.begin();it!= other->kp_seg_.end();++it)
        {                         
            auto seg = (*it).apply_offset(length_); 
            kp_seg_.push_back(seg);
        }

        for(auto it = other->curs_seg_.begin();it!= other->curs_seg_.end();++it)
        {                         
            auto seg = (*it).apply_offset(length_); 
            curs_seg_.push_back(seg);
        }

        if(speed_seg_)
        {
            for(auto it = other->speed_seg_->begin();it!= other->speed_seg_->end();++it)
            {                         
                auto seg = (*it).apply_offset(length_); 
                speed_seg_->push_back(seg);
            }
        }

    }

    void append(const traj_data& other) 
    {
        const auto length_ = length();

        for(auto it = other.kp_seg_.begin();it!= other.kp_seg_.end();++it)
        {                         
            auto seg = (*it).apply_offset(length_); 
            kp_seg_.push_back(seg);
        }

        for(auto it = other.curs_seg_.begin();it!= other.curs_seg_.end();++it)
        {                         
            auto seg = (*it).apply_offset(length_); 
            curs_seg_.push_back(seg);
        }

        if(speed_seg_ && other.speed_seg_)
        for(auto it = other.speed_seg_->begin();it!= other.speed_seg_->end();++it)
        {                         
            auto seg = (*it).apply_offset(length_); 
            speed_seg_->push_back(seg);
        }
        

    }

    void append(double len, const cg::point_3& pos,const cg::quaternion& orien, optional<double> speed)  override
    {
        kp_seg_.back().insert(make_pair<>(len,pos));
        curs_seg_.back().insert(make_pair<>(len,orien));
        if(speed)
            (*speed_seg_).back().insert(make_pair<>(len,*speed));

    }

    double length() const
    {
        return  kp_seg_.size()>0?kp_seg_.back().length():0;
    }
	
	double base_length() const
	{
		return  kp_seg_.size()>0?kp_seg_.front().points().begin()->first:0;
	}

    inline const keypoints_t::value_type kp_value(double arg) /*const*/
    {   
        size_t ind = current_segment(arg);
        return kp_seg_.at(ind).value(arg);
    }

    inline curses_t::value_type curs_value(double arg) /*const*/
    { 
        size_t ind = current_segment(arg);
        return curs_seg_.at(ind).value(arg);
    }

    /*inline*/ boost::optional<speed_t::value_type> speed_value(double arg) /*const*/
    { 
        size_t ind = current_segment(arg);
        if(speed_seg_ && speed_seg_->size() > ind && speed_seg_->size() > 0 )
            return speed_seg_->at(ind).value(arg);
        else
            return boost::none;
    }

    std::vector<keypoints_t::value_type> extract_values() const
    {
        std::vector<keypoints_t::value_type> values;

        size_t size = 0;
        for(auto it = kp_seg_.begin();it!=kp_seg_.end();++it)
        {    
            size += (*it).extract_values().size();
        }

        values.reserve(size);
        for(auto it = kp_seg_.begin();it!=kp_seg_.end();++it)
        {    
            auto kp_s = (*it).extract_values();
            for(auto it2 = kp_s.begin();it2!=kp_s.end();++it2)
                values.push_back(*it2);
        }

        return values;
    }

    double cur_len() const { return curr_pos_ ;}
    void   set_cur_len(double curr_len = 0.0) { curr_pos_ = curr_len;}



private:

    size_t current_segment(double curr_len)
    {
        for(auto it = kp_seg_.begin();it!=kp_seg_.end();++it)
        {
            if(curr_len <= (*it).length())
                return  std::distance(kp_seg_.begin(),it);
        }

        return std::distance(kp_seg_.begin(),kp_seg_.end()) - 1;
    }

    static int fill(keypoints_t& kp,curses_t& cr,double q[3], double x, void* /*user_data*/)
    {
        auto p = cg::point_2(q[0],q[1]);
        cr.insert(std::make_pair(x,(90 - cg::rad2grad()*q[2])));
        kp.insert(std::make_pair(x,p));

        return 0;
    }        


    double          curr_pos_;

};



} //ns fms