#pragma once

//
// Local includes
//

#include "av/part_sys/part_system.h"

#include "av/Fx.h"
#include "BoundInfo.h"

//
// Local namespaces
//

namespace avFx
{

	using namespace av::part_sys;

	//
	// Dust special effect
	//

	struct landing_dust_sfx_data /*: node_data*/
	{
		struct dust_event
		{
			float timestamp;
			float speed_val;
			cg::point_3f pos, vel, left;

			dust_event(float t, cg::point_3f const & p, cg::point_3f const & v) : timestamp(t), speed_val(cg::norm(v)), pos(p), vel(v), left(speed_val * normalized_safe(cg::point_3f(0, 0, 1) ^ v)) {}
			bool operator < (const dust_event & second) const { return timestamp < second.timestamp; }
		};
		std::multiset<dust_event> events;
	};


	static const float landing_dust_lifetime_min = 2.2f;
	static const float landing_dust_lifetime_max = 3.1f;


	//
	// LandingDustFx class
	// Implements landing dust effect
	//

	class LandingDustFx : public osg::Geode
		                , public LandingDustSfxNode
	{

	public:

		// constructor
		LandingDustFx();


		//
		// Base interface
		//


	public:

		// special culling method
		void cull( osg::NodeVisitor * pNV );

	private:
		void						 _createArrays();
		void                         _clearArrays();
		void                         _createGeometry();

	private: // particles_effect_info

		virtual float getMaxParticleLifetime() const override { return landing_dust_lifetime_max; }
		virtual bool  isQueueEmpty() const override { return !emitter_.get_queue().size(); }	

	private: // LandingDustFx

		void makeContactDust( float timestamp, cg::point_3f const & contact_pos, cg::point_3f const & contact_speed ) override
		{
			data_.events.insert(landing_dust_sfx_data::dust_event(timestamp, contact_pos, contact_speed));
		}

	private:

		// data
		landing_dust_sfx_data data_;

		// cpu part queue
		struct cpu_particle : base_cpu_particle
		{
			cg::point_3f cur_vel;
			cg::point_4f factors;
			cpu_particle( cg::point_3f const & sp, float lt, float age, cg::point_3f const & sv, cg::point_4f const & factors )
				: base_cpu_particle(sp, lt, age), cur_vel(sv), factors(factors) {}
		};
		typedef cpu_part_queue<cpu_particle> dust_cpu_queue;
		sfx_pos_time_emitter<dust_cpu_queue> emitter_;

		// gpu part queue
#pragma pack(push, 1)
		struct gpu_particle
		{
			cg::point_4f pos_time_unit;
			cg::point_4f factors;
		};
#pragma pack(pop)
#if 0
		gpu_part_drawer<gpu_particle> gpu_queue_;
#endif

		// clip data
		bound_info                    bvol_;

		osg::ref_ptr<osg::Vec4Array>  pos_time_unit_;
		osg::ref_ptr<osg::Vec4Array>  factors_;

		// drawing data
		osg::ref_ptr<osg::DrawArrays> _drawArrays;
		osg::ref_ptr<osg::Uniform>    _effectData;

	private:

		// AABB data for clipping
		osg::BoundingBox	          _aabbEllipsoid;

		osg::Geometry *               _geom;
	};

}