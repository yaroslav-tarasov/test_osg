
#include "stdafx.h"
#include <stdio.h>
using namespace std;
#include "geometry/curve.h"


typedef cg::curve_t<cg::point_2> keypoints_t;

int main_curves(int argc, char** argv )
{
    keypoints_t kp;
    keypoints_t::value_type prev_p(0,0);
    double dist = 0;
    for(int i =0;i<20;++i)
    {
        auto p = keypoints_t::value_type((double)i,(double)i*3);
        dist +=cg::distance(p,prev_p);
        kp.points().insert(std::make_pair(dist,p));
        prev_p = p;
    }

    keypoints_t::value_type val = kp.value(3.55);

    dist = 0;
    auto p_prev = kp.points().begin()->second;
    BOOST_FOREACH(const auto &p, kp.points())
    {
        dist +=cg::distance(p.second,p_prev);
        p_prev = p.second;
    }

    val = kp.value(100.0);

    return 0;
}

AUTO_REG(main_curves)