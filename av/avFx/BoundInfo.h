#pragma once

struct bound_info
{
	enum bound_type
	{
		has_aabb,
		has_obb,
		has_sphere
	} bvol_type;

	cg::aabb_clip_data aabb;
	cg::obb_clip_data obb;
	cg::sphere_clip_data sphere;

	bound_info( bound_type type = has_aabb ) : bvol_type(type) {}
};