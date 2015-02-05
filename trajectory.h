#pragma once


#include "dubins.h"
#include "geometry/curve.h"
#include "curve2.h"

namespace fms
{

struct trajectory
{   
    typedef cg::curve_t<cg::point_2>           keypoints_t;
    typedef cg::curve_t<double>                curses_t;

    trajectory(const decart_position& begin_pos,decart_position const& end_pos,double radius, double step = 1)
        :  curr_pos_(0)
    {
        double q0[] = { begin_pos.pos.x,begin_pos.pos.y,cg::grad2rad()*(90 - begin_pos.orien.get_course()) };
        double q1[] = { end_pos.pos.x,end_pos.pos.y,cg::grad2rad()*(90 - end_pos.orien.get_course() )};

        dubins_init( q0, q1, radius, &path_);
        dubins_path_sample_many( &path_,std::bind(fill,std::ref(kpts_),std::ref(crs_),sp::_1,sp::_2,sp::_3), step, nullptr);
    }
    
    void append(const trajectory &other) 
    {
        kpts_.append(other.kpts_);
        crs_.append(other.crs_);
    }

    const keypoints_t& get() const {return kpts_;};
    keypoints_t get()  {return kpts_;};

    const curses_t& get_curses() const {return crs_;};
    curses_t get_curses()              {return crs_;};
    
    virtual ~trajectory() {}
            
    inline double cur_len() const { return curr_pos_ ;}
    void   set_cur_len(double curr_len = 0.0) { curr_pos_ = curr_len;}

private:
    static int fill(keypoints_t& kp,curses_t& cr,double q[3], double x, void* user_data)
    {
        auto p = cg::point_2(q[0],q[1]);
        kp.points().insert(std::make_pair(x,p));
        cr.points().insert(std::make_pair(x,(90 - cg::rad2grad()*q[2])));
        return 0;
    }        

    DubinsPath    path_;
    keypoints_t   kpts_;
    curses_t       crs_;
    double    curr_pos_;
};

 typedef polymorph_ptr<trajectory> trajectory_ptr;

}