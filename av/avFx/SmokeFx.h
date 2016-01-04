#pragma once

//
// Local includes
//

#include "av/part_sys/part_system.h"

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

//
//  SmokeFx interface
//

// base info
struct particles_effect_info
{
	virtual float getMaxParticleLifetime() const = 0;
	virtual bool  isQueueEmpty() const = 0;
};

// Smoke
struct SmokeSfxNode : particles_effect_info
{
	virtual void  setIntensity( float inten ) = 0;
	virtual float getIntensity() const = 0;

	virtual void setEmitWorldDir( cg::point_3f const & dir ) = 0;
	virtual cg::point_3f const & getEmitWorldDir() const = 0;

	virtual void setEmitterWorldSpeed( cg::point_3f const & speed ) = 0;
	virtual cg::point_3f const & setEmitterWorldSpeed() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};


//
// Smoke special effect
//

struct smoke_sfx_data /*: node_data*/
{
	float intensity, factor;
	cg::point_3f emit_dir;
	cg::point_3f emitter_speed;

	smoke_sfx_data() : intensity(0), factor(1.f), emit_dir(0.f, 0.f, 1.f) {}
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

    class SmokeFx : public osg::Geode
				  ,	public SmokeSfxNode
    {

    public:

        // constructor
        SmokeFx();


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

		virtual float getMaxParticleLifetime() const override { return smoke_lifetime_max; }
		virtual bool  isQueueEmpty() const override { return !emitter_.get_queue().size(); }	

	private: // smoke_sfx_node

		virtual void                 setIntensity( float inten ) override;
		virtual float                getIntensity() const override { return data_.intensity; }

		virtual void                 setEmitWorldDir( cg::point_3f const & dir ) override { data_.emit_dir = dir; }
		virtual cg::point_3f const & getEmitWorldDir() const override { return data_.emit_dir; }

		virtual void                 setEmitterWorldSpeed( cg::point_3f const & speed ) override { data_.emitter_speed = speed; }
		virtual cg::point_3f const & setEmitterWorldSpeed() const override { return data_.emitter_speed; }

		virtual void                 setFactor( float factor ) override { data_.factor = cg::bound(factor, 0.0f, 10.f); }
		virtual float                getFactor() const override { return data_.factor; }

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
		osg::ref_ptr<osg::DrawArrays> m_DrawArrays;

    private:

        // AABB data for clipping
        osg::BoundingBox					  m_aabbEllipsoid;

		osg::Geometry *                       geom_;
    };

}