
//
// Module includes
//

#include <stdafx.h>

#include <osg/Texture2D>

#include "av/avCore/HalfTypeHeader.h"

#include "av/avCore/avCore.h"
#include "av/avScene/Scene.h"
//#include "svSea/Sea.h"

#include "av/avWeather/PrecipitationBase.h"

//
// Module namespaces
//

using namespace avWeather;

//
// Module local types
//

// describe half array
struct tagVertexData
{
    // 4 half-floats
    avCore::half x, y, z, t;

    // less operator for OSG Array instancing
    inline bool operator < (const tagVertexData & second) const
    {
        if (x < second.x) return true;
        else if (x > second.x) return false;
        else if (y < second.y) return true;
        else if (y > second.y) return false;
        else if (z < second.z) return true;
        else if (z > second.z) return false;
        else return (t < second.t);
    }
    // conversion operator for some acceptors
    operator osg::Vec4f() const
    {
        return osg::Vec4(x, y, z, t);
    }
};
// array instancing
typedef osg::TemplateArray<tagVertexData, osg::Array::ArrayType, 4, GL_HALF_FLOAT_ARB> Half4Array;
// damned acceptor specialization to make it compiling
inline void Half4Array::accept(unsigned int index, osg::ValueVisitor & vv) { return; }

//
// Base precipitation interface methods
//

// constructor
PrecipitationBase::PrecipitationBase(const char * szMaterialName, size_t nParticlesNumber, const osg::Vec3f & vBoxHalfSize, float fMeanLifeTime/* = 10.f*/)
    : m_bActive(false)
    , m_nParticlesNumber(nParticlesNumber)
    , m_vBoxHalf(vBoxHalfSize)
    , m_fIntensity(0.0f)
    , m_fWindSpeed(0.0f)
{
    // base set up
    setUseDisplayList(false);
    setUseVertexBufferObjects(true);
    setDataVariance(osg::Object::STATIC);
    setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback());
    


    //
    // create state set
    //

    osg::StateSet * pCurStateSet = getOrCreateStateSet();

    // setup shader defines (scroll scales and so on)
    avCore::Database::ShaderDefineVector cShaderDefines;

    // full box shader definition
    {
        const osg::Vec3f vBoxFull = m_vBoxHalf * 2.0f;
        std::ostringstream stream;
        stream << "vec3(" << vBoxFull.x() << ", " << vBoxFull.y() << ", " << vBoxFull.z() << ")";
        cShaderDefines.push_back(avCore::Database::ShaderDefine("SCROLL_FULL", stream.str().c_str()));
    }
    // full-box rcp shader definition
    {
        const osg::Vec3f vBoxFullRcp = osg::Vec3f(0.5f / m_vBoxHalf.x(), 0.5f / m_vBoxHalf.y(), 0.5f / m_vBoxHalf.z());
        std::ostringstream stream;
        stream << "vec3(" << vBoxFullRcp.x() << ", " << vBoxFullRcp.y() << ", " << vBoxFullRcp.z() << ")";
        cShaderDefines.push_back(avCore::Database::ShaderDefine("SCROLL_RCP", stream.str().c_str()));
    }

    // setup shader
    osg::Program * pCurProgram = new osg::Program;
    pCurProgram->setName(szMaterialName);
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader(std::string(szMaterialName).append(".vs").c_str(), &cShaderDefines, osg::Shader::VERTEX  ));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader(std::string(szMaterialName).append(".gs").c_str(), NULL           , osg::Shader::GEOMETRY));
    pCurProgram->addShader(avCore::GetDatabase()->LoadShader(std::string(szMaterialName).append(".fs").c_str(), NULL           , osg::Shader::FRAGMENT));
    pCurProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pCurProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pCurProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

    // bind shader
    pCurStateSet->setAttribute(pCurProgram);

    // setup texture for point quads
    pCurStateSet->setTextureAttribute(0, avCore::GetDatabase()->LoadTexture(std::string(szMaterialName).append(".dds").c_str(), osg::Texture::CLAMP_TO_EDGE));

    // box position
    m_uniformScrollLBN = new osg::Uniform("ScrollLBN", osg::Vec3f());
    pCurStateSet->addUniform(m_uniformScrollLBN.get());

    // LTP box offset
    m_uniformWP2LTPBoxOffset = new osg::Uniform("LTPOffset", osg::Vec2f());
    pCurStateSet->addUniform(m_uniformWP2LTPBoxOffset.get());

    // wind vector
    m_uniformWindVec = new osg::Uniform("WindVector", osg::Vec3f());
    pCurStateSet->addUniform(m_uniformWindVec.get());

    // render data
    m_uniformRenderData = new osg::Uniform("RenderData", osg::Vec4f());
    pCurStateSet->addUniform(m_uniformRenderData.get());

    //
    // Construct geometry
    //

    // attach draw call command
    m_dipDrawArrays = new osg::DrawArrays(osg::PrimitiveSet::POINTS);
    addPrimitiveSet(m_dipDrawArrays.get());

    // create vertex array
    Half4Array * aBoxPointsPos = new Half4Array(m_nParticlesNumber);
    // speed deviation factors
    osg::Vec4ubArray * aSpeedFactors = new osg::Vec4ubArray(m_nParticlesNumber);

    // fill every vertex point
    avCore::RandomNumber rndGen;
    for (size_t i = 0; i < m_nParticlesNumber; ++i)
    {
        // random lifetime
        float fLifeTime = rndGen.random_dev(fMeanLifeTime, 0.3f * fMeanLifeTime);
        // get random position
        osg::Vec3f vRandPos;
        vRandPos.x() = rndGen.random_unit_signed() * m_vBoxHalf.x();
        vRandPos.y() = rndGen.random_unit_signed() * m_vBoxHalf.y();
        vRandPos.z() = rndGen.random_unit_signed() * m_vBoxHalf.z();
        // save it
        tagVertexData & curVertex = aBoxPointsPos->at(i);
        curVertex.x = vRandPos.x();
        curVertex.y = vRandPos.y();
        curVertex.z = vRandPos.z();
        curVertex.t = fLifeTime;
        // speed deviation factors
        osg::Vec4ub & curSpeedFactors = aSpeedFactors->at(i);
        curSpeedFactors[0] = rndGen.random_8bit();
        curSpeedFactors[1] = rndGen.random_8bit();
        curSpeedFactors[2] = rndGen.random_8bit();
        curSpeedFactors[3] = rndGen.random_8bit();
    }

    // set vertex array
    aBoxPointsPos->setDataVariance(osg::Object::STATIC);
    setVertexArray(aBoxPointsPos);

    // set factors array
    static const size_t
        idxSpeedFactorsBinding = 5;
    aSpeedFactors->setDataVariance(osg::Object::STATIC);
    setVertexAttribBinding(idxSpeedFactorsBinding, osg::Geometry::BIND_PER_VERTEX);
    setVertexAttribNormalize(idxSpeedFactorsBinding, GL_TRUE);
    setVertexAttribArray(idxSpeedFactorsBinding, aSpeedFactors);


    // exit
    return;
}

//
// Base precipitation control
//

// set precipitation intensity
void PrecipitationBase::SetIntensity(float fIntensity)
{
    // save intensity with extra-clamping near zero
    m_fIntensity = cg::clamp01(fIntensity);

    // set active flag
    static const float g_fIntensityThreshold = 0.0001f;
    m_bActive = (m_fIntensity >= g_fIntensityThreshold);

    // set current particles number
    m_dipDrawArrays->setCount(m_fIntensity * m_nParticlesNumber);
}

// set wind direction and force
void PrecipitationBase::SetWind(const osg::Vec2f & vWindDir, float fForce)
{
    // calculate 3D-wind vector
    m_vWindDir = osg::Vec3f(vWindDir, 0.f) * fForce;

    // save it to uniform
    m_uniformWindVec->set(m_vWindDir);
}

//
// Camera-dependent control
//

// set camera position
void PrecipitationBase::SetViewerPos(const osg::Vec3f & vPos, const osg::Vec3f & vDir)
{
    // offset to box center from viewer
    const osg::Vec3f
        vDirBoxed = vDir * (0.5f + 0.5f / osg::maximum(osg::maximum(fabs(vDir[0]), fabs(vDir[1])), fabs(vDir[2]))),
        vDirOffset = osg::Vec3f(vDirBoxed[0] * m_vBoxHalf[0], vDirBoxed[1] * m_vBoxHalf[1], vDirBoxed[2] * m_vBoxHalf[2]);

    // calculate weather scroll LBN
    osg::Vec3f vWeatherLBNPos = vPos + vDirOffset - m_vBoxHalf;

    FIXME(Sea);
	// get sea data
    //avSea::Sea * pSea = avScene::GetScene()->getSea();
    //if (pSea)
    //{
    //    // try to make weather running only above sea (with 2 meters offset just for the case)
    //    vWeatherLBNPos.z() = osg::maximum(vWeatherLBNPos.z(), pSea->getTide() - 2.0f);
    //}

    // save it to uniform
    m_uniformScrollLBN->set(vWeatherLBNPos);
	
	
	FIXME(GetCoordinateSystem);

    // also save current LTP offset in weather plane
    osg::Vec2d vLTPOffsetInWP(0.0f, 0.0f); // = avCore::GetCoordinateSystem()->GetLTPOffsetInWeatherPlane();
    // make it to be boxed
    vLTPOffsetInWP.x() = fmod(vLTPOffsetInWP.x(), 2.0 * m_vBoxHalf.x());
    vLTPOffsetInWP.y() = fmod(vLTPOffsetInWP.y(), 2.0 * m_vBoxHalf.y());
    // and save it
    m_uniformWP2LTPBoxOffset->set(osg::Vec2f(vLTPOffsetInWP));
}

// set camera speed
void PrecipitationBase::SetCameraVelocity(const osg::Vec3f & vCamVel)
{
    m_vCamVel = vCamVel;
}

//
// Rendering related controls
//

// set rendering data
void PrecipitationBase::SetRenderData(float fScrClarity, float fMagicIllum)
{
    // save it to uniform
    m_uniformRenderData->set(osg::Vec4f(-fScrClarity, fMagicIllum, m_fIntensity, 0.f));
}

//
// Some callbacks for weather
//

// get intensity current
float PrecipitationBase::GetIntensity() const
{
    return m_fIntensity;
}

// get active state
bool PrecipitationBase::IsActive() const
{
    return m_bActive;
}
