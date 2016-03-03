//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avFx/LandingDustFx.h"

#include "utils/materials.h"

//
// Module namespaces
//

using namespace avFx;


//
// Local cloud constructor
//

// constructor
LandingDustFx::LandingDustFx()
{
	setName("LandingDustFx");
     
	_createGeometry();

	//
	// create state set
	//

	osg::StateSet * pCurStateSet = getOrCreateStateSet();

	// setup render-bin details
	pCurStateSet->setNestRenderBins(false);
	pCurStateSet->setRenderBinDetails(RENDER_BIN_PARTICLE_EFFECTS, "DepthSortedBin");

	// setup blending
	osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	pCurStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

	osg::BlendEquation* pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
	pCurStateSet->setAttributeAndModes(pBlendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

	// enable depth test but disable depth write
	osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
	pCurStateSet->setAttribute(pDepth);

#if 0
	pCurStateSet->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE,osg::StateAttribute::ON);  
#endif

	// enable depth clamping to avoid cutting
	pCurStateSet->setMode(GL_DEPTH_CLAMP_NV, osg::StateAttribute::ON);

	pCurStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	// setup shader
	osg::Program * pCurProgram = new osg::Program;
	pCurProgram->setName("LandingDustFx");
	pCurProgram->addShader(avCore::GetDatabase()->LoadShader("LandingDustFx.vs", NULL, osg::Shader::VERTEX  ));
	pCurProgram->addShader(avCore::GetDatabase()->LoadShader("LandingDustFx.gs", NULL, osg::Shader::GEOMETRY));
	pCurProgram->addShader(avCore::GetDatabase()->LoadShader("LandingDustFx.fs", NULL, osg::Shader::FRAGMENT));
	pCurProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
	pCurProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
	pCurProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);


	// bind shader
	pCurStateSet->setAttribute(pCurProgram);

	// uniforms

	_effectData = new osg::Uniform("effect_data"  , osg::Vec4(0.0,0.0,0.0,0.0));
    pCurStateSet->addUniform( _effectData.get() );

	pCurStateSet->addUniform( new osg::Uniform("SmokeAtlas"  , 0) );

	pCurStateSet->addUniform( new osg::Uniform("ViewLightMap"  , BASE_LM_TEXTURE_UNIT) );
	pCurStateSet->addUniform( new osg::Uniform("envTex"        , BASE_ENV_TEXTURE_UNIT) );

	osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
	pCurStateSet->setTextureAttributeAndModes( BASE_LM_TEXTURE_UNIT, creators::getTextureHolder().getLightMapTexture().get(), value ); 
	pCurStateSet->setTextureAttributeAndModes( BASE_ENV_TEXTURE_UNIT, creators::getTextureHolder().getEnvTexture().get()    , value );

	// textures

	// setup texture for point quads
	pCurStateSet->setTextureAttribute(0, avCore::GetDatabase()->LoadTexture("images/sfx/smoke_puff_atlas.dds", osg::Texture::REPEAT));




	setCullCallback(Utils::makeNodeCallback(this, &LandingDustFx::cull));
	// exit
	return;
}


void LandingDustFx::_createArrays()
{
	factors_            = new osg::Vec4Array();
	_geom->setVertexAttribArray(1, factors_.get(),osg::Array::BIND_PER_VERTEX);
	_geom->setVertexAttribNormalize(1, false);

	pos_time_unit_->setPreserveDataType(true);
	factors_->setPreserveDataType(true);

	// draw arrays command, that would be executed
	_drawArrays = new osg::DrawArrays(osg::PrimitiveSet::POINTS);
	_drawArrays->setFirst(0);
	_drawArrays->setCount(0);

	_geom->addPrimitiveSet(_drawArrays.get());
}

void LandingDustFx::_clearArrays()
{
	pos_time_unit_->clear();
	factors_->clear();
}

void LandingDustFx::_createGeometry()
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

// update
void LandingDustFx::cull( osg::NodeVisitor * pNV )
{
	osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
	avAssert(pCV);

	const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();

	const avCore::Environment::IlluminationParameters & cIlluminationParameters = avCore::GetEnvironment()->GetIlluminationParameters();
	const avCore::Environment::EnvironmentParameters  & cEnvironmentParameters  = avCore::GetEnvironment()->GetEnvironmentParameters();

	_effectData->set(osg::Vec4(cIlluminationParameters.Illumination, 0.0, 0.0, 0.0));

	// particles updater
	cg::point_3f const wind_vec(cEnvironmentParameters.WindSpeed * cEnvironmentParameters.WindDirection);
	auto const & cpu_updater = [&wind_vec]( cpu_particle & part, float dt )
	{
		part.cur_vel *= exp(-2.0f * dt);
		part.cur_pos() += (wind_vec + part.cur_vel) * dt;
	};

	// update current
	static const float break_sfx_dist = std::numeric_limits<float>::infinity();
	const float sim_time_cur = pNV->getFrameStamp()->getSimulationTime();
	emitter_.trace_and_update(sim_time_cur, cg::point_3f(), break_sfx_dist, cpu_updater);

	// need to emit?
	while (true)
	{
		auto it = data_.events.begin();
		if (it == data_.events.end() || it->timestamp > sim_time_cur)
			break;

		// inject parts
		const auto & dust = *it;

		// new particles emitter
		auto const & cpu_emit_new = [&dust]( cg::point_3f const & /*wp*/, float /*emit_timestamp*/, float tfe, simplerandgen & rnd )->cpu_particle
		{
			const float lt = rnd.random_range(landing_dust_lifetime_min, landing_dust_lifetime_max);

			const auto vel_fwd = dust.vel * rnd.random_range(0.7f, 0.9f);
			const auto vel_right = dust.left * rnd.random_range(0.25f, 0.35f);
			const float angle = rnd.random_unit() * cg::pif;
			const auto start_vel = sin(angle) * vel_fwd + cos(angle) * vel_right;

			cg::point_4f randoms;
			randoms.x = rnd.random_unit();
			randoms.y = rnd.random_unit();
			randoms.z = rnd.random_unit();
			randoms.w = rnd.random_unit();

			return LandingDustFx::cpu_particle(dust.pos, lt, tfe, start_vel, randoms);
		};
		emitter_.inject_particles(dust.timestamp, 16, cpu_emit_new, cpu_updater);

		data_.events.erase(it);
	}

	_clearArrays();

	// feed it to gpu
	auto const & cpu_queue = emitter_.get_queue();
	pos_time_unit_->resize(cpu_queue.size()); 
	factors_->resize(cpu_queue.size());

	size_t i =0;
	for (auto part = cpu_queue.begin(), it_end = cpu_queue.end(); part != it_end; ++part, ++i)
	{
		// result_aabb |= part->cur_pos();
		auto cpu_p = *part;
		pos_time_unit_->at(i).set(cpu_p.cur_pos().x,cpu_p.cur_pos().y,cpu_p.cur_pos().z,cpu_p.t());
		factors_->at(i).set(float(cpu_p.factors.x), float(cpu_p.factors.y), float(cpu_p.factors.z), float(cpu_p.factors.w));
	}

	pos_time_unit_->dirty();
	factors_->dirty();

	_drawArrays->setCount(cpu_queue.size());
}
