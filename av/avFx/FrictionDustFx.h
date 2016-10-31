#pragma once

//
// Local includes
//

#include "av/part_sys/part_system.h"

#include "av/Fx.h"
#include "BoundInfo.h"


//
// Dust special effect
//

struct friction_dust_sfx_data /*: node_data*/
{
	bool active;
	cg::point_3f emitter_velocity;

	friction_dust_sfx_data() : active(false) {}
};


static const float dust_lifetime_min = 4.5f;
static const float dust_lifetime_max = 6.5f;

//
// Local namespaces
//

namespace avFx
{

	using namespace av::part_sys;

    //
    // FrictionDustFx class
    // Implements friction dust special effect
    //

    class FrictionDustFx : public osg::Geode
				  ,	public FrictionDustSfxNode
    {

    public:

        // constructor
        FrictionDustFx();


        //
        // Base interface
        //


    public:

        // special culling method
        // void cull  ( osgUtil::CullVisitor * pCV );
		void cull( osg::NodeVisitor * pNV );

	private:
		void						 _createArrays();
		void                         _clearArrays();
        void                         _createGeometry();

	private: // particles_effect_info

		float getMaxParticleLifetime() const override { return dust_lifetime_max; }
		bool  isQueueEmpty() const override { return !emitter_.get_queue().size(); }	

	private: // FrictionDustSfxNode

		void                 setContactFlag( bool flag ) override;
		bool                 getContactFlag() const override { return data_.active; }

		void                 setEmitterWorldVelocity( cg::point_3f const & velocity ) override;
		cg::point_3f const & getEmitterWorldVelocity() const override { return data_.emitter_velocity; }

	private:

		// data
		friction_dust_sfx_data data_;
		cg::point_3f           velocity_left;
		float                  speed_val_;

		// cpu part queue
		struct cpu_particle : base_cpu_particle
		{
			cg::point_3f cur_vel;
			float factor;
			cg::colorab randoms;
			cg::point_3f c_vel;
			cpu_particle( cg::point_3f const & sp, float lt, float age, cg::point_3f const & sv, float f, cg::colorab const & rand, cg::point_3f const & c_vel ) :
			base_cpu_particle(sp, lt, age), cur_vel(sv), factor(f), randoms(rand), c_vel(c_vel) {}
		};
		typedef cpu_part_queue<cpu_particle> frictiondust_cpu_queue;
		sfx_pos_time_emitter<frictiondust_cpu_queue> emitter_;

		// gpu part queue
#pragma pack(push, 1)
		struct gpu_particle
		{
			cg::point_4f pos_time_unit;
			cg::point_3f factor_dummies;
			cg::colorab  randoms;
		};
#pragma pack(pop)
#if 0
		gpu_part_drawer<gpu_particle> gpu_queue_;
#endif

		// clip data
		bound_info                    bvol_;

		osg::ref_ptr<osg::Vec4Array>  pos_time_unit_;
		osg::ref_ptr<osg::Vec3Array>  factor_dummies_;
		osg::ref_ptr<osg::Vec4Array>  randoms_;
		// drawing data
		osg::ref_ptr<osg::DrawArrays> _drawArrays;

    private:

        // AABB data for clipping
        osg::BoundingBox					  _aabbEllipsoid;

		osg::Geometry *                       _geom;
    };

}