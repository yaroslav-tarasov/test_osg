#pragma once

#include <random>

struct std_simple_randgen
{
	double random_range(const double from, const double to)
	{
		return std::bind(
			std::uniform_real_distribution<>(from, to),
			std::default_random_engine( rd() ))();
	}

	inline cg::point_3 inside_unit_sphere () 
	{
		return cg::point_3(random_range(-1., 1.),random_range(-1., 1.),random_range(-1., 1.));
	}


	std::random_device rd;

};

#if 0
double random_range (double min_val, double max_val)
{ 
	return min_val + ( static_cast<double> (std::rand()) / 32767.0 * ( max_val - min_val + 1 ) );
}
#endif

