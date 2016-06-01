#pragma once

//
// Local includes
//

#include "av/part_sys/part_system.h"

#include "Fx.h"
#include "BoundInfo.h"

//
// Smoke special effect
//

struct smoke_sfx_data /*: node_data*/
{
	float            intensity,
		                factor;
	cg::point_3f      emit_pos;
	cg::point_3f      emit_dir;
	cg::point_3f emitter_speed;

	smoke_sfx_data() : intensity(0), factor(1.f), emit_dir(0.f, 0.f, 1.f), emit_pos(cg::point_3f()) {}
};


static const float smoke_lifetime_min = 4.0f;
static const float smoke_lifetime_max = 8.5f;

//
// Local namespaces
//

namespace avFx
{

	using namespace av::part_sys;

    //
    // SmokeFx class
    // Implements smoke effect
    //

    class FireFx : public osg::Geode
				  ,	public SmokeSfxNode
    {

    public:

        // constructor
        FireFx();


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

		virtual float getMaxParticleLifetime() const override { return smoke_lifetime_max; }
		virtual bool  isQueueEmpty() const override { return !emitter_.get_queue().size(); }	

	private: // SmokeSfxNode

		void                 setIntensity( float inten ) override;
		float                getIntensity() const override { return data_.intensity; }

		void                 setEmitWorldDir( cg::point_3f const & dir ) override { data_.emit_dir = dir; }
		cg::point_3f const & getEmitWorldDir() const override { return data_.emit_dir; }

		void                 setEmitWorldPos( cg::point_3f const & emit_pos ) override { data_.emit_pos = emit_pos; }
		cg::point_3f const & getEmitWorldPos() const override { return data_.emit_pos; }

		void                 setEmitterWorldSpeed( cg::point_3f const & speed ) override { data_.emitter_speed = speed; }
		cg::point_3f const & getEmitterWorldSpeed() const override { return data_.emitter_speed; }

		void                 setFactor( float factor ) override { data_.factor = cg::bound(factor, 0.0f, 10.f); }
		float                getFactor() const override { return data_.factor; }

	private:

		// data
		smoke_sfx_data data_;

		// cpu part queue
		struct cpu_particle : base_cpu_particle
		{
			float start_time;
			cg::point_3f start_vel;
			float factor;
			cg::colorab randoms;
			cpu_particle( cg::point_3f const & sp, float st, float lt, float age, cg::point_3f const & sv, float f, cg::colorab const & rand ) :
			base_cpu_particle(sp, lt, age), start_time(st), start_vel(sv), factor(f), randoms(rand) {}
		};

		typedef cpu_part_queue<cpu_particle>    smoke_cpu_queue;
		sfx_pos_time_emitter<smoke_cpu_queue>   emitter_;

		// gpu part queue
#pragma pack(push, 1)
		struct gpu_particle
		{
			cg::point_4f pos_start_time;
			cg::point_3f lifetimercp_factor_dummy;
			cg::colorab randoms;
		};
#pragma pack(pop)
		gpu_part_drawer<gpu_particle> gpu_queue_;

		// clip data
		bound_info                    bvol_;

		osg::ref_ptr<osg::Vec4Array>  pos_start_time_;
		osg::ref_ptr<osg::Vec3Array>  lifetimercp_factor_;
		osg::ref_ptr<osg::Vec4Array>  randoms_;
		// drawing data
		osg::ref_ptr<osg::DrawArrays> _drawArrays;

    private:

        // AABB data for clipping
        osg::BoundingBox			 _aabbEllipsoid;
		osg::Geometry *              _geom;
    };

}