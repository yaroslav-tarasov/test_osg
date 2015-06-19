#include "stdafx.h"

#include <osg/BlendFunc>
#include <osg/Geometry>

#include "av/Scene.h"

#include "FogLayer.h"

//
// Global namespace
//

using namespace avSky;

//
// Fog sky dome layer class implementation
//

// constructor
FogLayer::FogLayer(osg::Group * pScene)
    : m_fRealVisDist(40000.0f)
{
    // create fog uniform for the whole scene
    osg::StateSet * pSceneSS = pScene->getOrCreateStateSet();
    _sceneFogUniform = new osg::Uniform("SceneFogParams", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_sceneFogUniform.get());

    // create fog uniform for reflections pass
    // sea is created first, so this code is somehow safe
    avScene::Scene * pRealScenePtr = static_cast<avScene::Scene *>(pScene);
    if (pRealScenePtr->getSea())
    {
        osg::StateSet * pReflSS = pRealScenePtr->getSea()->getMainReflectionGroup()->getOrCreateStateSet();
        _reflFogUniform = new osg::Uniform("SceneFogParams", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
        pReflSS->addUniform(_reflFogUniform.get());
    }

    // state set composing
    _buildStateSet();
    // geometry creation
    _createGeometry();
}

// set fog params
void FogLayer::setFogParams( const osg::Vec3f & vFogColor, float fFogDensity )
{
    // for sky
    _skyFogUniform->set(osg::Vec4f(vFogColor, fFogDensity));

    // for scene
    const float fFogExp2Coef = utils::ConvertTraineeDensityToExpSceneFog(fFogDensity, &m_fRealVisDist);
    // save uniform
    _sceneFogUniform->set(osg::Vec4f(vFogColor, fFogExp2Coef));

    // for reflections
    if (_reflFogUniform.valid())
    {
        const float fReflFogExp2Coef = utils::ConvertTraineeDensityToExpReflFog(fFogDensity, NULL);
        // save uniform
        _reflFogUniform->set(osg::Vec4f(vFogColor, fReflFogExp2Coef));
    }
}

// set underwater color
void FogLayer::SetUnderWaterColor( const osg::Vec3f & cUnderWaterColor )
{
    _underWaterLowColorUniform->set(cUnderWaterColor);
}

//
// Some geometry related functions
//

// create state set
void FogLayer::_buildStateSet()
{
    osg::StateSet * sset = getOrCreateStateSet();

    // set render bin
    sset->setRenderBinDetails(RENDER_BIN_SKYFOG, "RenderBin");
    sset->setNestRenderBins(false);

    // create clarity shader
    osg::ref_ptr<osg::Program> cFogLayerProg = new osg::Program();
    cFogLayerProg->setName("FogLayerShader");
    cFogLayerProg->addShader(/*utils::GetDatabase()->LoadShader("SkyFog.vs")*/
        osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("SkyFog.vs")));
    cFogLayerProg->addShader(/*utils::GetDatabase()->LoadShader("SkyFog.fs")*/
        osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("SkyFog.fs")));
    sset->setAttribute(cFogLayerProg.get());

    // set appropriate mix blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc();
    pBlendFunc->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    sset->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

    // create fog uniform for the sky layer
    _skyFogUniform = new osg::Uniform("SkyFogParams", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    sset->addUniform(_skyFogUniform.get());

    // create underwater color for the sky layer
    _underWaterLowColorUniform = new osg::Uniform("UnderWaterLowColor", osg::Vec3f(1.f, 1.f, 1.f));
    sset->addUniform(_underWaterLowColorUniform.get());

    // done, exit
    return;
}

// create fake layer geometry
void FogLayer::_createGeometry()
{
    // dummy bounding box callback
    osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();

    // create OSG geode with 1 drawable node
    setCullingActive(false);
    setDataVariance(osg::Object::STATIC);

    // create tetrahedron around viewer (just to fill the whole volume)
    osg::Geometry * box_geometry = new osg::Geometry;
    box_geometry->setUseDisplayList(true);
    box_geometry->setDataVariance(osg::Object::STATIC);
    box_geometry->setComputeBoundingBoxCallback(pDummyBBCompute);

    const float fSqrt3 = sqrtf(3.0f);

    // create its' vertex
    osg::Vec3Array * paBoxPointsPos = new osg::Vec3Array;
    paBoxPointsPos->resize(4);
    paBoxPointsPos->at(0).set(0.f, 0.f, +2.f);
    paBoxPointsPos->at(1).set(-2.0f * fSqrt3, 0.f, -1.0f);
    paBoxPointsPos->at(2).set(fSqrt3, -3.0f, -1.0f);
    paBoxPointsPos->at(3).set(fSqrt3, +3.0f, -1.0f);
    // set vertex array
    paBoxPointsPos->setDataVariance(osg::Object::STATIC);
    box_geometry->setVertexArray(paBoxPointsPos);

    // draw elements command, that would be executed
    // volume is made looking inside
    osg::DrawElementsUShort * paBoxDrawElem = new osg::DrawElementsUShort(GL_TRIANGLES, 12);
    paBoxDrawElem->at(0)  = 0;
    paBoxDrawElem->at(1)  = 2;
    paBoxDrawElem->at(2)  = 1;
    paBoxDrawElem->at(3)  = 0;
    paBoxDrawElem->at(4)  = 3;
    paBoxDrawElem->at(5)  = 2;
    paBoxDrawElem->at(6)  = 0;
    paBoxDrawElem->at(7)  = 1;
    paBoxDrawElem->at(8)  = 3;
    paBoxDrawElem->at(9)  = 1;
    paBoxDrawElem->at(10) = 2;
    paBoxDrawElem->at(11) = 3;

    // add DIP
    paBoxDrawElem->setDataVariance(osg::Object::STATIC);
    box_geometry->addPrimitiveSet(paBoxDrawElem);

    // all is done, so add box to this geode
    addDrawable(box_geometry);
}
