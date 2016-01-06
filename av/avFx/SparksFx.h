#pragma once

//
// Local includes
//

#include "av/part_sys/part_system.h"

#include "Fx.h"
#include "BoundInfo.h"

//
// Local namespaces
//

namespace avFx
{

	using namespace av::part_sys;

	//
	// Smoke special effect
	//

	struct sparks_sfx_data /*: node_data*/
	{
		bool active;
		float factor;
		cg::point_3f emitter_speed;

		sparks_sfx_data() : active(false), factor(0.075f) {}
	};

	static const float spark_lifetime_min = 1.45f;
	static const float spark_lifetime_max = 1.95f;


    //
    // SparksFx class
    // Implements smoke effect
    //

    class SparksFx : public osg::Geode
				   ,	public SparksSfxNode
    {

    public:

        // constructor
        SparksFx();


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

		virtual float getMaxParticleLifetime() const override { return spark_lifetime_max; }
		virtual bool  isQueueEmpty() const override { return !emitter_.get_queue().size(); }	

	private: // SparksFx
		
		virtual void                 setContactFlag( bool flag ) override;
		virtual bool                 getContactFlag() const override { return data_.active; }

		virtual void                 setEmitterWorldSpeed( cg::point_3f const & speed ) override;
		virtual cg::point_3f const & getEmitterWorldSpeed() const override { return data_.emitter_speed; }

		virtual void                 setFactor( float factor ) override { data_.factor = cg::bound(factor, 0.0f, 10.f); }
		virtual float                getFactor() const override { return data_.factor; }

	private:

		// data
		sparks_sfx_data data_;
		cg::point_3f speed_left;
		float speed_val_;

		// cpu part queue
		struct cpu_particle : base_cpu_particle
		{
			cg::point_3f cur_vel;
			cg::point_3f prev_pos;
			cpu_particle( cg::point_3f const & sp, float lt, float age, cg::point_3f const & sv ) : base_cpu_particle(sp, lt, age), cur_vel(sv), prev_pos(sp) {}
		};
		typedef cpu_part_queue<cpu_particle> sparks_cpu_queue;
		sfx_pos_time_emitter<sparks_cpu_queue> emitter_;

		// gpu part queue
#pragma pack(push, 1)
		struct gpu_particle
		{
			cg::point_4f pos_time_unit;
			cg::point_3f prev_pos;
			cg::colorab  randoms;
		};
#pragma pack(pop)
		gpu_part_drawer<gpu_particle> gpu_queue_;

		// clip data
		bound_info                    bvol_;

		osg::ref_ptr<osg::Vec4Array>  pos_time_unit_;
		osg::ref_ptr<osg::Vec3Array>  prev_pos_;
		osg::ref_ptr<osg::Vec4Array>  randoms_;
		// drawing data
		osg::ref_ptr<osg::DrawArrays> _drawArrays;
		osg::ref_ptr<osg::Uniform>    _effectData;

    private:

        // AABB data for clipping
        osg::BoundingBox	      m_aabbEllipsoid;

		osg::Geometry *               _geom;
    };

}