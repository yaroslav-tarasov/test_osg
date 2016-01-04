//
// Module includes
//

#include <stdafx.h>

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"

#include "av/avFx/SmokeFx.h"

#include "utils/materials.h"

//
// Module namespaces
//

using namespace avFx;


//
// Local cloud constructor
//

// constructor
SmokeFx::SmokeFx()
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
    pCurProgram->setName("SmokeFx");
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("SmokeFx.vs", NULL, osg::Shader::VERTEX  ));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("SmokeFx.gs", NULL, osg::Shader::GEOMETRY));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader("SmokeFx.fs", NULL, osg::Shader::FRAGMENT));
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
    pCurStateSet->setTextureAttribute(0, avCore::GetDatabase()->LoadTexture("images/sfx/smoke_atlas.dds", osg::Texture::REPEAT));

	
	setCullCallback(utils::makeNodeCallback(this, &SmokeFx::cull));
    // exit
    return;
}


void SmokeFx::_createArrays()
{
	lifetimercp_factor_ = new osg::Vec3Array();
	geom_->setVertexAttribArray(1, lifetimercp_factor_.get(),osg::Array::BIND_PER_VERTEX);
	geom_->setVertexAttribNormalize(1, false);

	randoms_            = new osg::Vec4Array();
	geom_->setVertexAttribArray(2, randoms_.get(),osg::Array::BIND_PER_VERTEX);
	geom_->setVertexAttribNormalize(2, false);

	pos_start_time_->setPreserveDataType(true);
	lifetimercp_factor_->setPreserveDataType(true);
	randoms_->setPreserveDataType(true);

	// draw arrays command, that would be executed
	m_DrawArrays = new osg::DrawArrays(osg::PrimitiveSet::POINTS);
	m_DrawArrays->setFirst(0);
	m_DrawArrays->setCount(0);

	geom_->addPrimitiveSet(m_DrawArrays.get());
}

void SmokeFx::_clearArrays()
{
	pos_start_time_->clear();
	lifetimercp_factor_->clear();
	randoms_->clear();
}

void SmokeFx::_createGeometry()
{
	// dummy bounding box callback
	osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

	// create OSG geode with 1 drawable node
	setCullingActive(false);
	setDataVariance(osg::Object::DYNAMIC);

	// create tetrahedron around viewer (just to fill the whole volume)
	geom_ = new osg::Geometry;
	geom_->setComputeBoundingBoxCallback(pDummyBBCompute);

	pos_start_time_ = new osg::Vec4Array;
	pos_start_time_->setDataVariance(osg::Object::DYNAMIC);
	// set vertex array

	geom_->setVertexArray(pos_start_time_);

	geom_->setUseDisplayList(false);
	geom_->setUseVertexBufferObjects(true);
	geom_->setDataVariance(osg::Object::DYNAMIC);
	
	addDrawable(geom_);

	_createArrays();

}

void SmokeFx::setIntensity( float inten )
{
	inten = abs(inten);
	if (cg::eq_zero(data_.intensity) || cg::eq_zero(inten))
		emitter_.reset_emanation_last();

	data_.intensity = inten;
}

// update
void SmokeFx::cull( osg::NodeVisitor * pNV )
{

	osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(pNV);
	avAssert(pCV);

	/*auto const & wp = pUV->get_father_scene()->get_env_weather()->get_weather();*/
	FIXME(Need some wind);
	const cg::point_3f wind_dir(0.0f,1.0f,0.0f);
	const float wind_speed = 1.5f;

	// particles updater
	cg::point_3f const wind_vec(/*wp.*/wind_dir * /*wp.*/wind_speed);
	auto const & cpu_updater = [&wind_vec]( cpu_particle & part, float dt )
	{
		const float & t_unit = part.t();
		const float & t = part.get_age();
		const float mega_age_func = t_unit * (t_unit * (0.333333f * t_unit - 1.0f) + 1.0f);
		const cg::point_3f velocity_impact = cg::point_3f(part.start_vel.x, part.start_vel.y, part.start_vel.z + 35.0f) * (mega_age_func * t);
		part.start_pos() += wind_vec * dt;
		part.cur_pos() = part.start_pos() + velocity_impact;
	};

	const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();

	// update current
	static const float break_sfx_dist = 80.f;
	force_log fl;       
	LOG_ODS_MSG( "pNV->getFrameStamp()->getSimulationTime() " << pNV->getFrameStamp()->getSimulationTime()<< "\n");
	emitter_.trace_and_update(pNV->getFrameStamp()->getSimulationTime(), cg::point_3f(0.0,0.0,0.0)/*from_osg_vector3(mWorldToView.getTrans())*//*pUV->get_current_model_world().translation()*/, break_sfx_dist, cpu_updater);

	// new particles emitter
	const float factor_val = data_.factor;
	auto const & cpu_emit_new = [&factor_val]( cg::point_3f const & wp, float emit_timestamp, float tfe, simplerandgen & rnd )->cpu_particle
	{
		const float lt_particle = rnd.random_range(smoke_lifetime_min, smoke_lifetime_max);

		cg::colorab randoms;
		randoms.r = rnd.random_8bit();
		randoms.g = rnd.random_8bit();
		randoms.b = rnd.random_8bit();
		randoms.a = rnd.random_8bit();

		cg::point_3f start_vel;
		start_vel.x = rnd.random_unit_signed() * (factor_val * 5.0f);
		start_vel.y = rnd.random_unit_signed() * (factor_val * 5.0f);
		start_vel.z = rnd.random_unit() * (factor_val * 2.5f);

		return SmokeFx::cpu_particle(wp, emit_timestamp, lt_particle, tfe, start_vel, factor_val, randoms);
	};
	// particles emission
	static const float break_time_dist = 2.f;
	emitter_.emit_new_particles(data_.intensity, 1.25f / cg::max(data_.factor, 1.0f), break_time_dist, cpu_emit_new, cpu_updater);
	
	_clearArrays();
	
	// feed it to gpu
#if 0
	gpu_queue_.fill<smoke_cpu_queue, gpu_particle>(emitter_.get_queue(), bvol_.aabb, data_.factor * 8.f,
		[this](gpu_particle & gpu_p, cpu_particle const & cpu_p)
	{
		gpu_p.pos_start_time = cg::point_4f(cpu_p.cur_pos(), cpu_p.start_time);
		gpu_p.lifetimercp_factor_dummy = cg::point_3f(cpu_p.lifetime_inv(), cpu_p.factor, 0.f);
		gpu_p.randoms = cpu_p.randoms;
	});
#endif

	auto const & cpu_queue = emitter_.get_queue();
	pos_start_time_->resize(cpu_queue.size()); 
    lifetimercp_factor_->resize(cpu_queue.size());
    randoms_->resize(cpu_queue.size());
	
	
	size_t i =0;
	for (auto part = cpu_queue.begin(), it_end = cpu_queue.end(); part != it_end; ++part, ++i)
	{
		// result_aabb |= part->cur_pos();
		auto cpu_p = *part;
		pos_start_time_->at(i).set(cpu_p.cur_pos().x,cpu_p.cur_pos().y,cpu_p.cur_pos().z,cpu_p.start_time);
		lifetimercp_factor_->at(i).set(cpu_p.lifetime_inv(), cpu_p.factor, 0.f);
		randoms_->at(i).set(float(cpu_p.randoms.r)/255.0f,float(cpu_p.randoms.g)/255.0f,float(cpu_p.randoms.b)/255.0f,float(cpu_p.randoms.a)/255.0f); 
	}

	pos_start_time_->dirty();
	lifetimercp_factor_->dirty();
	randoms_->dirty();

	m_DrawArrays->setCount(cpu_queue.size());
}
