#pragma once


#include "dubins.h"
#include "geometry/curve.h"
#include "curve2.h"

class TrajectoryDrawer;

namespace fms
{

struct trajectory
{   
    friend  class TrajectoryDrawer;

    typedef cg::curve_t<cg::point_2>           keypoints_t;
    typedef cg::curve_t<double>                   curses_t;
    typedef std::vector<keypoints_t>         kp_segments_t;
    typedef std::vector<curses_t>            cr_segments_t;

    trajectory(const decart_position& begin_pos,decart_position const& end_pos,double radius, double step = 1)
        :  curr_pos_(0)
        ,  length_  (0)
    {
        double q0[] = { begin_pos.pos.x,begin_pos.pos.y,cg::grad2rad()*(90 - begin_pos.orien.get_course()) };
        double q1[] = { end_pos.pos.x,end_pos.pos.y,cg::grad2rad()*(90 - end_pos.orien.get_course() )};
        keypoints_t         kpts_;
        curses_t             crs_;
        dubins_init( q0, q1, radius, &path_);
        dubins_path_sample_many( &path_,std::bind(fill,std::ref(kpts_),std::ref(crs_),sp::_1,sp::_2,sp::_3), step, nullptr);
        kp_seg_.push_back(kpts_);
        curs_seg_.push_back(crs_);
        length_ = kpts_.length();
    }
    
    void append(const trajectory &other) 
    {
        size_t size = 0;
        for(auto it = other.kp_seg_.begin();it!=other.kp_seg_.end();++it)
        {                         
            auto seg = (*it).apply_offset(length_); 
            kp_seg_.push_back(seg);
	    }

		for(auto it = other.curs_seg_.begin();it!=other.curs_seg_.end();++it)
		{                         
			auto seg = (*it).apply_offset(length_); 
			curs_seg_.push_back(seg);
		}

        // kp_seg_.insert(kp_seg_.end(),other.kp_seg_.begin(), other.kp_seg_.end());
        // curs_seg_.insert(curs_seg_.end(),other.curs_seg_.begin(),other.curs_seg_.end());

        {    
            length_ = kp_seg_.back().length();
        }

		std::stringstream cstr;

		cstr << std::setprecision(8) 
			<< "length_:  "         << length_
			<< "\n" ;

		OutputDebugString(cstr.str().c_str());
    }
    
    inline double length() const
    {
        return length_;
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
            // values.insert(values.end(),(*it).extract_values().begin(), (*it).extract_values().end());
            auto kp_s = (*it).extract_values();
            for(auto it2 = kp_s.begin();it2!=kp_s.end();++it2)
                values.push_back(*it2);
        }
            
        return values;
    }

    virtual ~trajectory() {}
            
    inline double cur_len() const { return curr_pos_ ;}
    void   set_cur_len(double curr_len = 0.0) { curr_pos_ = curr_len;}

private:
    
    size_t current_segment(double curr_len)
    {
        for(auto it = kp_seg_.begin();it!=kp_seg_.end();++it)
        {
             if(curr_len <= (*it).length())
                 return  std::distance(kp_seg_.begin(),it);
        }
        return 0;
    }

    static int fill(keypoints_t& kp,curses_t& cr,double q[3], double x, void* user_data)
    {
        auto p = cg::point_2(q[0],q[1]);
        cr.insert(std::make_pair(x,(90 - cg::rad2grad()*q[2])));
        kp.insert(std::make_pair(x,p));

        return 0;
    }        

    DubinsPath          path_;
    //keypoints_t         kpts_;
    //curses_t             crs_;
    kp_segments_t     kp_seg_;
    cr_segments_t   curs_seg_;
    double          curr_pos_;
    double            length_;
};

 typedef polymorph_ptr<trajectory> trajectory_ptr;

}