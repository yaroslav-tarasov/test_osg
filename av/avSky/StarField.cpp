#include "stdafx.h"

#include <osg/BlendEquation>
#include <osg/Geometry>
#include <osg/Geode>

#include "StarField.h"
#include "star_data.h"
#include "utils/random.h"
#include "av/Utils.h"

using namespace avSky;

// constructor
StarField::StarField()
{
    setNodeMask(~65536);
    
    //setNodeMask(NODE_STARFIELD_MASK);

    // parse all stars
    for (const char **sptr = star_data; *sptr; sptr++)
    {
        std::stringstream ss(*sptr);
        m_aStars.push_back(StarData(ss));
    }

    // create geode
    _buildGeometry();
}

// control stars
void StarField::setIlluminationFog( float fIllum, float fFog )
{
    const float fUnitScale = cg::lerp_clamp(fIllum, 0.0f, 1.0f, 1.0f, 0.0f);

    // get overall intensity scale
    m_starsUniform->set(osg::Vec4f(
        fUnitScale,
        cg::slerp01(cg::lerp_clamp(fFog, 0.3f, 0.7f, 1.0f, 0.0f)),
        0.f,
        0.f));
}

// create geometry
void StarField::_buildGeometry()
{
    // create geode
    m_geodeStars = new osg::Geode();
    m_geodeStars->setDataVariance(osg::Object::STATIC);
    addChild(m_geodeStars);

    // create geometry
    osg::ref_ptr<osg::Geometry> pStarsGeom = new osg::Geometry();
    pStarsGeom->setUseDisplayList(false);
    pStarsGeom->setUseVertexBufferObjects(true);
    pStarsGeom->setDataVariance(osg::Object::STATIC);
    m_geodeStars->setCullingActive(false);
    m_geodeStars->addDrawable(pStarsGeom.get());

    // create its' vertices
    osg::ref_ptr<osg::Vec4Array> pStarsVerts = new osg::Vec4Array();
    pStarsVerts->setDataVariance(osg::Object::STATIC);
    pStarsGeom->setVertexArray(pStarsVerts);

    // create colors
    osg::ref_ptr<osg::Vec4ubArray> pStarsColors = new osg::Vec4ubArray();
    pStarsVerts->setDataVariance(osg::Object::STATIC);
    pStarsGeom->setColorArray(pStarsColors);
    pStarsGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    // fill them
    utils::RandomNumber rndGen;
    pStarsVerts->resize(m_aStars.size());
    pStarsColors->resize(m_aStars.size());
    for (unsigned i = 0; i < m_aStars.size(); ++i)
    {
        const cg::point_3f vStarDir(utils::celestialUnitVector(m_aStars[i].right_ascension, m_aStars[i].declination));
        pStarsVerts->at(i) = osg::Vec4(vStarDir.x, vStarDir.y, vStarDir.z, 1.6f - m_aStars[i].magnitude * 0.15f);
        // random color
        cg::point_3f cStarRandomColor;
        cStarRandomColor.x = rndGen.random_unit();
        cStarRandomColor.x *= cStarRandomColor.x;
        cStarRandomColor.x *= cStarRandomColor.x;
        cStarRandomColor.y = (1.0f - cStarRandomColor.x) * rndGen.random_unit();
        cStarRandomColor.y *= cStarRandomColor.y;
        cStarRandomColor.y *= cStarRandomColor.y;
        float fRest = cStarRandomColor.x + cStarRandomColor.y;
        fRest *= fRest;
        fRest *= fRest;
        cStarRandomColor.z = 1.0f - fRest;
        FIXME("lerp01?");
        cStarRandomColor = cg::lerp01( cg::point_3f(0.87f, 0.92f, 0.95f), cg::point_3f(1.0f, 1.0f, 1.0f),cStarRandomColor.z);
        pStarsColors->at(i) = osg::Vec4ub(cStarRandomColor.x * 255, cStarRandomColor.y * 255, cStarRandomColor.z * 255, 255);
    }

    // create indices
    osg::ref_ptr<osg::DrawArrays> pStarsIndices = new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, m_aStars.size());
    pStarsIndices->setDataVariance(osg::Object::STATIC);
    pStarsGeom->addPrimitiveSet(pStarsIndices.get());


    // setup state-set
    osg::StateSet * pSS = m_geodeStars->getOrCreateStateSet();
    pSS->setRenderBinDetails(RENDER_BIN_STARS, "RenderBin");
    pSS->setNestRenderBins(false);

    // create stars program
    osg::ref_ptr<osg::Program> pStarsProgram = new osg::Program();
    pStarsProgram->setName("Stars");
    pStarsProgram->addShader(/*utils::GetDatabase()->LoadShader("Stars.vs")*/
        osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("Stars.vs")));
    pStarsProgram->addShader(/*utils::GetDatabase()->LoadShader("Stars.fs")*/
        osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("Stars.fs")));
    pSS->setAttribute(pStarsProgram);

    // set appropriate additive blending
    osg::BlendEquation * pBlendEq = new osg::BlendEquation();
    pBlendEq->setEquation(osg::BlendEquation::RGBA_MAX);
    pSS->setAttributeAndModes(pBlendEq, osg::StateAttribute::ON);
    pSS->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
    pSS->setMode(0x8642/*GL_PROGRAM_POINT_SIZE*/, osg::StateAttribute::ON);

    // create starts intensity control uniform
    m_starsUniform = new osg::Uniform("StarsIntensityParams", osg::Vec4f());
    pSS->addUniform(m_starsUniform);
}

//
// Star data
//

// constructor
StarField::StarData::StarData( std::stringstream & ss )
{
    getline(ss, name, ',');
    std::string buff;
    getline(ss, buff, ',');
    std::stringstream(buff) >> right_ascension;
    getline(ss, buff, ',');
    std::stringstream(buff) >> declination;
    getline(ss, buff, '\n');
    std::stringstream(buff) >> magnitude;
}
