
#include "stdafx.h"
#include "fms/dubins.h"
#include <stdio.h>
using namespace std;
#include "geometry/curve.h"

int printConfiguration(double q[3], double x, void* user_data) {
	printf("%f, %f, %f, %f\n", q[0], q[1], q[2], x);
	return 0;
}


typedef cg::curve_t<cg::point_2> keypoints_t;

static int fill(keypoints_t& kp,double q[3], double x, void* user_data)
{
    auto p = cg::point_2(q[0],q[1]);
    kp.points().insert(std::make_pair(x,p));
    return 0;
}



int main_dubins(int argc, char** argv )
{
	keypoints_t  sample_points;
	
	double q0[] = { 0,0,0 };
	double q1[] = { 4,4,3.142 };
	double turning_radius = 1.0;
	DubinsPath path;
	dubins_init( q0, q1, turning_radius, &path);
	dubins_path_sample_many( &path, printConfiguration, 0.1, NULL);
	
    // We need work around for MSVC 2010 internal error
    auto fn = [&sample_points](double q[3], double x, void* user_data)->int {
        auto p = cg::point_2(q[0],q[1]);
        sample_points.points().insert(std::make_pair(x,p));	
        return 0;
    };

	dubins_path_sample_many( &path,std::bind(fill,std::ref(sample_points),sp::_1,sp::_2,sp::_3), 0.1, NULL);
    
    keypoints_t::value_type val = sample_points.value(3.55);
    
    double dist = 0;
    auto p_prev = sample_points.points().begin()->second;
    BOOST_FOREACH(const auto &p, sample_points.points())
    {
        dist +=cg::distance(p.second,p_prev);
        p_prev = p.second;
    }

	return 0;
}

AUTO_REG(main_dubins)