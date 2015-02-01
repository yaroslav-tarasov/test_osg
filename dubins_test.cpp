
#include "stdafx.h"
#include "dubins.h"
#include <stdio.h>
using namespace std;
#include "geometry/curve.h"



int printConfiguration(double q[3], double x, void* user_data) {
	printf("%f, %f, %f, %f\n", q[0], q[1], q[2], x);
	return 0;
}

typedef cg::curve_t<double> keypoints_t;

int main_dubins(int argc, char** argv )
{
	keypoints_t  sample_points;
	
	double q0[] = { 0,0,0 };
	double q1[] = { 4,4,3.142 };
	double turning_radius = 1.0;
	DubinsPath path;
	dubins_init( q0, q1, turning_radius, &path);
	dubins_path_sample_many( &path, printConfiguration, 0.1, NULL);
	
	std::function<int (double q[3], double t, void* user_data)> fn;
	//= ([=](double q[3], double x, void* user_data)->int{
	//	// q[0], q[1], q[2], x
	//	// sample_points.points().insert(std::make_pair(q[0],q[1]));	
	//	return 0;
	//});

	dubins_path_sample_many( &path,lambda_cast<DubinsPathSamplingCallback>::Cast([=](double q[3], double x, void* user_data)->int{
	    // q[0], q[1], q[2], x
		// sample_points.points().insert(std::make_pair(q[0],q[1]));	
		return 0;
	}) , 0.1, NULL);
	

	//for (auto it = traj_->points().begin(), end = traj_->points().end(); it != end; ++it)
	//	pnts.insert(std::make_pair(it->dist, it->dyn_state.pos.height));

	return 0;
}