//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avFx/SparksFx.h"

#include "utils/materials.h"

//
// Module namespaces
//

using namespace avFx;


//
// Local cloud constructor
//

// constructor
SparksFx::SparksFx()
{
    setName("SparksFx");

	_createGeometry();

    //
    // create state set
    //

    osg::StateSet * pCurStateSet = getOrCreateStateSet();

    // setup render-bin details
    pCurStateSet->setNestRenderBins(false);
    pCurStateSet->setRenderBinDetails(RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin");

    // setup blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ZERO);
    pCurStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);
	
	osg::BlendEquation* pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
	pCurStateSet->setAttributeAndModes(pBlendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // enable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, true);
    pCurStateSet->setAttribute(pDepth);

	pCurStateSet->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE,osg::StateAttribute::ON);  
#if 0
	// enable depth clamping to avoid cutting
	pCurStateSet->setMode(GL_DEPTH_CLAMP_NV, osg::StateAttribute::ON);
#endif

    // disable cull-face just for the case
    pCurStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // setup shader
    osg::Program * pCurProgram = new osg::Program;
    pCurProgram->setName("SparksFx");
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("SparksFx.vs", NULL, osg::Shader::VERTEX  ));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("SparksFx.gs", NULL, osg::Shader::GEOMETRY));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("SparksFx.fs", NULL, osg::Shader::FRAGMENT));
    pCurProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pCurProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pCurProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);


    // bind shader
    pCurStateSet->setAttribute(pCurProgram);
 
	// uniforms

	_effectData = new osg::Uniform("effect_data"  , osg::Vec4(0.0,0.0,0.0,0.0));
	
	osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
	
	pCurStateSet->addUniform( new osg::Uniform("envTex"        , BASE_ENV_TEXTURE_UNIT) );
	pCurStateSet->setTextureAttributeAndModes( BASE_ENV_TEXTURE_UNIT, creators::getTextureHolder().getEnvTexture().get()    , value );

	pCurStateSet->addUniform( _effectData.get() );
	
	
	setCullCallback(utils::makeNodeCallback(this, &SparksFx::cull));
    // exit
    return;
}


void SparksFx::_createArrays()
{
	prev_pos_ = new osg::Vec3Array();
	_geom->setVertexAttribArray(1, prev_pos_.get(),osg::Array::BIND_PER_VERTEX);
	_geom->setVertexAttribNormalize(1, false);

	randoms_            = new osg::Vec4Array();
	_geom->setVertexAttribArray(2, randoms_.get(),osg::Array::BIND_PER_VERTEX);
	_geom->setVertexAttribNormalize(2, false);

	pos_time_unit_->setPreserveDataType(true);
	prev_pos_->setPreserveDataType(true);
	randoms_->setPreserveDataType(true);

	// draw arrays command, that would be executed
	_drawArrays = new osg::DrawArrays(osg::PrimitiveSet::POINTS);
	_drawArrays->setFirst(0);
	_drawArrays->setCount(0);

	_geom->addPrimitiveSet(_drawArrays.get());
}

void SparksFx::_clearArrays()
{
	pos_time_unit_->clear();
	prev_pos_->clear();
	randoms_->clear();
}

void SparksFx::_createGeometry()
{
	// dummy bounding box callback
	osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

	// create OSG geode with 1 drawable node
	setCullingActive(false);
	setDataVariance(osg::Object::DYNAMIC);

	// create tetrahedron around viewer (just to fill the whole volume)
	_geom = new osg::Geometry;
	_geom->setComputeBoundingBoxCallback(pDummyBBCompute);

	pos_time_unit_ = new osg::Vec4Array;
	pos_time_unit_->setDataVariance(osg::Object::DYNAMIC);
	// set vertex array

	_geom->setVertexArray(pos_time_unit_);

	_geom->setUseDisplayList(false);
	_geom->setUseVertexBufferObjects(true);
	_geom->setDataVariance(osg::Object::DYNAMIC);
	
	addDrawable(_geom);

	_createArrays();

}

//
// sparks_sfx_node
//

void SparksFx::setContactFlag( bool flag )
{
	if (flag != data_.active)
		emitter_.reset_emanation_last();
	data_.active = flag;
}

void SparksFx::setEmitterWorldSpeed( cg::point_3f const & speed )
{
	data_.emitter_speed = speed;
	speed_left = normalized_safe(cg::point_3f(0, 0, 1) ^ speed);
	speed_val_ = cg::norm(data_.emitter_speed);
}

// update
void SparksFx::cull( osg::NodeVisitor * pNV )
{
	osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
	avAssert(pCV);
    
	const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();
	const osg::Matrixd mProjection  = *pCV->getProjectionMatrix();

	const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
	const avCore::Environment::EnvironmentParameters  & cEnvironmentParameters  = avCore::GetEnvironment()->GetEnvironmentParameters();

	_effectData->set(osg::Vec4(data_.factor * mProjection(0,0), cIlluminationParameters.Illumination, 1.f / pCV->getViewport()->width(), 0.0));

	// particles updater
    cg::point_3f const wind_vec(cEnvironmentParameters.WindSpeed * cEnvironmentParameters.WindDirection);
	auto const & cpu_updater = [&wind_vec]( cpu_particle & part, float dt )
	{
		part.prev_pos = part.cur_pos();

		part.cur_vel *= exp(-0.6f * dt);
		part.cur_vel.z -= 18.0f * dt;

		part.cur_pos() += (wind_vec + part.cur_vel) * dt;
		if (part.cur_pos().z <= 0.f && part.cur_vel.z <= 0.f)
		{
			part.cur_pos().z = -part.cur_pos().z;
			part.cur_vel.z = -0.8f * part.cur_vel.z;
			part.cur_vel *= 0.9f;
		}
	};

	// update current
	static const float break_sfx_dist = 30.f;
	emitter_.trace_and_update(pNV->getFrameStamp()->getSimulationTime(), cg::point_3f(150.0f,200.0,0.0)/*from_osg_vector3(mWorldToView.getTrans())*/, break_sfx_dist, cpu_updater);

	// need to emit?
	if (data_.active)
	{
		// decide about intensity
		float intensity_cur = cg::clamp(7.0f, 60.f, 0.f, 1.f)(speed_val_);
		intensity_cur *= 0.7f * intensity_cur + 0.3f;
		// new particles emitter
		auto const & cpu_emit_new = [this, intensity_cur]( cg::point_3f const & wp, float /*emit_timestamp*/, float tfe, simplerandgen & rnd )->cpu_particle
		{
			const unsigned spark_dir = rnd.random_32bit() & 3;
			const float lt = rnd.random_range(spark_lifetime_min, spark_lifetime_max);
			cg::point_3f vel = data_.emitter_speed;
			if (spark_dir < 2)
			{
				const float speed_sign = -1.f + 2.f * spark_dir;
				vel = vel * rnd.random_dev(1.1f, 0.15f) + speed_left * (speed_sign * rnd.random_dev(3.f + 4.f * intensity_cur, 2.f));
				vel.z = rnd.random_dev(3.f + 3.f * intensity_cur, 2.f);
			}
			else
			{
				vel = vel * rnd.random_dev(0.9f, 0.075f) + speed_left * rnd.random_dev(0.f, 1.f + 2.f * intensity_cur);
				vel.z = rnd.random_dev(2.f + 2.f * intensity_cur, 1.f);
			}
			return SparksFx::cpu_particle(wp, lt, tfe, vel);
		};
		// particles emission
		static const float break_time_dist = 1.f;
		emitter_.emit_new_particles(intensity_cur * 5.f, intensity_cur / cg::max(data_.factor, 1.f), break_time_dist, cpu_emit_new, cpu_updater);
	}

	_clearArrays();
	
	// feed it to gpu
	auto const & cpu_queue = emitter_.get_queue();
	pos_time_unit_->resize(cpu_queue.size()); 
    prev_pos_->resize(cpu_queue.size());
    randoms_->resize(cpu_queue.size());
	
	
	size_t i =0;
	for (auto part = cpu_queue.begin(), it_end = cpu_queue.end(); part != it_end; ++part, ++i)
	{
		// result_aabb |= part->cur_pos();
		auto cpu_p = *part;
		pos_time_unit_->at(i).set(cpu_p.cur_pos().x,cpu_p.cur_pos().y,cpu_p.cur_pos().z,cpu_p.t());
		prev_pos_->at(i).set(cpu_p.prev_pos.x, cpu_p.prev_pos.y, cpu_p.prev_pos.z);
	}

	pos_time_unit_->dirty();
	prev_pos_->dirty();
	randoms_->dirty();

	_drawArrays->setCount(cpu_queue.size());
}
