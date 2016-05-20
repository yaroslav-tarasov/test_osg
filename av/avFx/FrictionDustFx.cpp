//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avFx/FrictionDustFx.h"

#include "utils/materials.h"

//
// Module namespaces
//

using namespace avFx;


//
// Dust special effect constructor
//

// constructor
FrictionDustFx::FrictionDustFx()
{
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

	// enable depth clamping to avoid cutting
	pCurStateSet->setMode(GL_DEPTH_CLAMP_NV, osg::StateAttribute::ON);

#if 0
	// do not write alpha
	osg::ColorMask * pColorMask = new osg::ColorMask(true, true, true, false);
	pCurStateSet->setAttribute(pColorMask);
#endif

    // disable cull-face just for the case
    pCurStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // setup shader
    osg::Program * pCurProgram = new osg::Program;
    pCurProgram->setName("FrictionDustFx");
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("FrictionDustFx.vs", NULL, osg::Shader::VERTEX  ));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("FrictionDustFx.gs", NULL, osg::Shader::GEOMETRY));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("FrictionDustFx.fs", NULL, osg::Shader::FRAGMENT));
    pCurProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pCurProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pCurProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);


    // bind shader
    pCurStateSet->setAttribute(pCurProgram);
   


    // uniforms

   
	pCurStateSet->addUniform( new osg::Uniform("SmokeAtlas"  , 0) );
	
	pCurStateSet->addUniform( new osg::Uniform("ViewLightMap"  , BASE_LM_TEXTURE_UNIT) );
    pCurStateSet->addUniform( new osg::Uniform("envTex"        , BASE_ENV_TEXTURE_UNIT) );

	osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
	pCurStateSet->setTextureAttributeAndModes( BASE_LM_TEXTURE_UNIT, creators::getTextureHolder().getLightMapTexture().get(), value ); 
	pCurStateSet->setTextureAttributeAndModes( BASE_ENV_TEXTURE_UNIT, creators::getTextureHolder().getEnvTexture().get()    , value );
	
	// textures

    // setup texture for point quads
    pCurStateSet->setTextureAttribute(0, avCore::GetDatabase()->LoadTexture("images/sfx/smoke_puff_atlas.dds", osg::Texture::REPEAT));

	
	setCullCallback(Utils::makeNodeCallback(this, &FrictionDustFx::cull));
    // exit
    return;
}


void FrictionDustFx::_createArrays()
{
	factor_dummies_ = new osg::Vec3Array();
	_geom->setVertexAttribArray(1, factor_dummies_.get(),osg::Array::BIND_PER_VERTEX);
	_geom->setVertexAttribNormalize(1, false);

	randoms_            = new osg::Vec4Array();
	_geom->setVertexAttribArray(2, randoms_.get(),osg::Array::BIND_PER_VERTEX);
	_geom->setVertexAttribNormalize(2, false);

	pos_time_unit_->setPreserveDataType(true);
	factor_dummies_->setPreserveDataType(true);
	randoms_->setPreserveDataType(true);

	// draw arrays command, that would be executed
	_drawArrays = new osg::DrawArrays(osg::PrimitiveSet::POINTS);
	_drawArrays->setFirst(0);
	_drawArrays->setCount(0);

	_geom->addPrimitiveSet(_drawArrays.get());
}

void FrictionDustFx::_clearArrays()
{
	pos_time_unit_->clear();
	factor_dummies_->clear();
	randoms_->clear();
}

void FrictionDustFx::_createGeometry()
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
// FrictionDustSfxNode
//

void FrictionDustFx::setContactFlag( bool flag )
{
	if (flag != data_.active)
		emitter_.reset_emanation_last();
	data_.active = flag;
}

void FrictionDustFx::setEmitterWorldSpeed( cg::point_3f const & speed )
{
	data_.emitter_speed = speed;
	speed_left = normalized_safe(cg::point_3f(0, 0, 1) ^ speed);
	speed_val_ = cg::norm(data_.emitter_speed);
}

// update
void FrictionDustFx::cull( osg::NodeVisitor * pNV )
{

	osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
	avAssert(pCV);

	const avCore::Environment::EnvironmentParameters  & cEnvironmentParameters  = avCore::GetEnvironment()->GetEnvironmentParameters();

	// particles updater
	cg::point_3f const wind_vec(cEnvironmentParameters.WindSpeed * cEnvironmentParameters.WindDirection);
	auto const & cpu_updater = [&wind_vec]( cpu_particle & part, float dt )
	{
		part.cur_vel *= exp(-1.75f * dt);
		part.cur_pos() += (wind_vec + part.c_vel + part.cur_vel) * dt;
	};
	// update current
	static const float break_sfx_dist = 30.f;
	emitter_.trace_and_update(pNV->getFrameStamp()->getSimulationTime(),  cg::point_3f(0.0f,0.0,0.0)/*from_osg_vector3(mWorldToView.getTrans())*/, break_sfx_dist, cpu_updater);

	// need to emit?
	if (data_.active)
	{
		// decide about intensity
		float intensity_cur = cg::clamp(15.0f, 70.f, 0.f, 1.f)(speed_val_);
		intensity_cur *= 0.5f * intensity_cur + 0.5f;
		// new particles emitter
		auto const & cpu_emit_new = [this, intensity_cur]( cg::point_3f const & wp, float /*emit_timestamp*/, float tfe, simplerandgen & rnd )->cpu_particle
		{
			const float lt = rnd.random_range(dust_lifetime_min, dust_lifetime_max);

			cg::colorab randoms;
			randoms.r = rnd.random_8bit();
			randoms.g = rnd.random_8bit();
			randoms.b = rnd.random_8bit();
			randoms.a = rnd.random_8bit();

			cg::point_3f start_vel = this->data_.emitter_speed;

			cg::point_3f cvel;
			cvel.x += rnd.random_unit_signed() * (intensity_cur * 3.5f);
			cvel.y += rnd.random_unit_signed() * (intensity_cur * 3.5f);
			cvel.z += rnd.random_unit() * (intensity_cur * 1.0f);

			return FrictionDustFx::cpu_particle(wp, lt, tfe, start_vel, intensity_cur, randoms, cvel);
		};
		// particles emission
		static const float break_time_dist = 1.f;
		emitter_.emit_new_particles(0, 0.8f, break_time_dist, cpu_emit_new, cpu_updater);
	}

	// feed it to gpu
	FIXME("Масштаб?");
	static const float max_part_size = 20.f;
	_clearArrays();
	
	auto const & cpu_queue = emitter_.get_queue();
	pos_time_unit_->resize(cpu_queue.size()); 
    factor_dummies_->resize(cpu_queue.size());
    randoms_->resize(cpu_queue.size());
	
	
	size_t i =0;
	for (auto part = cpu_queue.begin(), it_end = cpu_queue.end(); part != it_end; ++part, ++i)
	{
		// result_aabb |= part->cur_pos();
		auto cpu_p = *part;
		pos_time_unit_->at(i).set(cpu_p.cur_pos().x,cpu_p.cur_pos().y,cpu_p.cur_pos().z,cpu_p.t());
		factor_dummies_->at(i).set( cpu_p.factor, 0.f, 0.f);
		randoms_->at(i).set(float(cpu_p.randoms.r)/255.0f,float(cpu_p.randoms.g)/255.0f,float(cpu_p.randoms.b)/255.0f,float(cpu_p.randoms.a)/255.0f); 
	}

    if(i>0)
    {
	    pos_time_unit_->dirty();
	    factor_dummies_->dirty();
	    randoms_->dirty();

	    _drawArrays->setCount(cpu_queue.size());
    }
}
