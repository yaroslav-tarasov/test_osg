#include "av/precompiled.h"

#include "Moon.h"
#include "av/avCore/Utils.h"

using namespace avSky;

// constructor
MoonSphere::MoonSphere()
{
    // create geode
    _buildGeometry();
}

// control
void MoonSphere::setDirection( float fAzimuth, float fElevation )
{
    const cg::point_3f vMoonDir = Utils::celestialUnitVector(fAzimuth, fElevation);
    m_moonVectorUniform->set(osg::Vec3f(vMoonDir.x, vMoonDir.y, vMoonDir.z));
}

// color
void MoonSphere::setMoonReflColor( const osg::Vec3f & cMoonColor )
{
    osg::Geometry * pMoonGeom = static_cast<osg::Geometry *>(getDrawable(0));
    if (pMoonGeom)
    {
        // pMoonGeom->setColorData(osg::Geometry::ArrayData(new osg::Vec3Array(1u, const_cast<osg::Vec3f *>(&cMoonColor)), osg::Geometry::BIND_OVERALL));
        pMoonGeom->setColorArray(new osg::Vec3Array(1u, const_cast<osg::Vec3f *>(&cMoonColor)), osg::Array::BIND_OVERALL);
    }
}

// create geometry
void MoonSphere::_buildGeometry()
{
    // create geode
    setDataVariance(osg::Object::STATIC);
    setCullingActive(false);

    // create geometry
    osg::ref_ptr<osg::Geometry> pMoonGeom = new osg::Geometry();
    pMoonGeom->setUseDisplayList(false);
    pMoonGeom->setUseVertexBufferObjects(true);
    pMoonGeom->setDataVariance(osg::Object::STATIC);
    addDrawable(pMoonGeom.get());

    // subdivide now
    static const unsigned
        g_uPhiSubd = 40,
        g_uThetaSubd = 25,
        g_uTotalVertsNum = g_uThetaSubd * g_uPhiSubd,
        g_uTotalQuadsNum = (g_uThetaSubd - 1) * (g_uPhiSubd - 1);

    // create its' vertices
    osg::ref_ptr<osg::Vec3Array> pMoonVerts = new osg::Vec3Array();
    pMoonVerts->setDataVariance(osg::Object::STATIC);
    pMoonGeom->setVertexArray(pMoonVerts);
    osg::ref_ptr<osg::Vec2Array> pMoonTexCoords = new osg::Vec2Array();
    pMoonTexCoords->setDataVariance(osg::Object::STATIC);
    pMoonGeom->setTexCoordArray(0, pMoonTexCoords);
    osg::ref_ptr<osg::Vec3Array> pMoonTangents = new osg::Vec3Array();
    //pMoonGeom->setVertexAttribBinding(5, osg::Geometry::BIND_PER_VERTEX);
    pMoonGeom->setVertexAttribArray(5, pMoonTangents.get(), osg::Array::BIND_PER_VERTEX);
    osg::ref_ptr<osg::Vec3Array> pMoonBiTangents = new osg::Vec3Array();
    //pMoonGeom->setVertexAttribBinding(6, osg::Geometry::BIND_PER_VERTEX);
    pMoonGeom->setVertexAttribArray(6, pMoonBiTangents.get(), osg::Array::BIND_PER_VERTEX);

    // fill them
    pMoonVerts->resize(g_uTotalVertsNum);
    pMoonTexCoords->resize(g_uTotalVertsNum);
    pMoonTangents->resize(g_uTotalVertsNum);
    pMoonBiTangents->resize(g_uTotalVertsNum);
    for (unsigned i = 0; i < g_uThetaSubd; ++i)
    {
        // theta parameter
        const float fTexV = i / (g_uThetaSubd - 1.0f);
        const float fTheta = -osg::PI_2 + osg::PI * fTexV;
        float sin_theta, cos_theta;
        cg::fsincos(fTheta, &sin_theta, &cos_theta);
        for (unsigned j = 0; j < g_uPhiSubd; ++j)
        {
            // phi parameter
            const float fTexU = j / (g_uPhiSubd - 1.0f);
            const float fPhi = 2.0f * osg::PI * fTexU;
            float sin_phi, cos_phi;
            cg::fsincos(fPhi, &sin_phi, &cos_phi);
            // fill everything
            const unsigned uVertIdx = i * g_uPhiSubd + j;
            const cg::point_3f vSphereDir(cos_phi * cos_theta, sin_phi * cos_theta, sin_theta);
            pMoonVerts->at(uVertIdx).set(vSphereDir.x, vSphereDir.y, vSphereDir.z);
            pMoonTexCoords->at(uVertIdx).set(fTexU, fTexV);
            pMoonTangents->at(uVertIdx).set(-sin_phi, cos_phi, 0.f);
            pMoonBiTangents->at(uVertIdx).set(-cos_phi * sin_theta, -sin_phi * sin_theta, cos_theta);
        }
    }

    // create indices
    osg::ref_ptr<osg::DrawElementsUShort> pMoonIndices = new osg::DrawElementsUShort(osg::PrimitiveSet::QUADS, g_uTotalQuadsNum * 4);
    pMoonIndices->setDataVariance(osg::Object::STATIC);
    for (unsigned i = 0; i < g_uThetaSubd - 1; ++i)
    {
        for (unsigned j = 0; j < g_uPhiSubd - 1; ++j)
        {
            const unsigned nQuadIdx = 4 * (i * (g_uPhiSubd - 1) + j);
            pMoonIndices->at(nQuadIdx + 0) = (i + 0) * g_uPhiSubd + (j + 0);
            pMoonIndices->at(nQuadIdx + 1) = (i + 0) * g_uPhiSubd + (j + 1);
            pMoonIndices->at(nQuadIdx + 2) = (i + 1) * g_uPhiSubd + (j + 1);
            pMoonIndices->at(nQuadIdx + 3) = (i + 1) * g_uPhiSubd + (j + 0);
        }
    }
    pMoonGeom->addPrimitiveSet(pMoonIndices.get());

    // setup state-set
    osg::StateSet * pSS = getOrCreateStateSet();
    pSS->setRenderBinDetails(RENDER_BIN_SUN_MOON, "RenderBin");
    pSS->setNestRenderBins(false);
    pSS->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // add bump texture
    osg::Texture2D * pMoonBumpTex = new osg::Texture2D(osgDB::readImageFile("Sky/moon_normal.dds",new osgDB::Options("")));//utils::GetDatabase()->LoadTexture("Sky/moon_normal.dds", osg::Texture::REPEAT);
    if (pMoonBumpTex)
    {
        pMoonBumpTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
        pMoonBumpTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
        pMoonBumpTex->setWrap(  osg::Texture::WRAP_R, osg::Texture::REPEAT );

        static const int iBumpTex = 0;
        pMoonBumpTex->setMaxAnisotropy(16.0f);
        pSS->setTextureAttribute(iBumpTex, pMoonBumpTex);
        pSS->addUniform(new osg::Uniform("MoonBumpTexture", iBumpTex));
    }

    // add color texture
    osg::Texture2D * pMoonColorTex = new osg::Texture2D(osgDB::readImageFile("Sky/moon_color.dds"));//utils::GetDatabase()->LoadTexture("Sky/moon_color.dds", osg::Texture::REPEAT);
    if (pMoonColorTex)
    {
        static const int iColorTex = 1;

        pMoonColorTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::REPEAT );
        pMoonColorTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::REPEAT );
        pMoonColorTex->setWrap(  osg::Texture::WRAP_R, osg::Texture::REPEAT );

        pMoonColorTex->setMaxAnisotropy(16.0f);
        pSS->setTextureAttribute(iColorTex, pMoonColorTex);
        pSS->addUniform(new osg::Uniform("MoonColorTexture", iColorTex));
    }

    // create moon program
    osg::ref_ptr<osg::Program> pMoonProgram = new osg::Program();
    pMoonProgram->setName("Moon");
    pMoonProgram->addShader(/*utils::GetDatabase()->LoadShader("Moon.vs")*/
        osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("Moon.vs")));
    pMoonProgram->addShader(/*utils::GetDatabase()->LoadShader("Moon.fs")*/
        osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("Moon.fs")));
    pSS->setAttribute(pMoonProgram);

    // set appropriate additive blending
    //osg::BlendFunc * pBlendFunc = new osg::BlendFunc();
    //pBlendFunc->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //pSS->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);
    osg::BlendEquation * pBlendEquation = new osg::BlendEquation();
    pBlendEquation->setEquation(osg::BlendEquation::RGBA_MAX);
    pSS->setAttributeAndModes(pBlendEquation, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // direction uniform
    m_moonVectorUniform = new osg::Uniform("MoonVector", osg::Vec3f(0.577350f, 0.577350f, 0.577350f));
    pSS->addUniform(m_moonVectorUniform);
}
