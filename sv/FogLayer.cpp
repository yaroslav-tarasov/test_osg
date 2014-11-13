#include "stdafx.h"

#include "FogLayer.h"
#include "../creators.h"

//
// Global namespace
//
//
// Static functions
//

static const double g_clearDayVisibility = 35000.0;
static const double g_totalFogVisibility = 400.0;
static const double g_fogPowRampCoef = 3.0;

//////////////////////////////////////////////////////////////////////////
static float convertTraineeDensityToExp2SceneFog( float fTraineeDensity, float * fVisDistPtr )
{
    // calculate real visibility distance somehow
    static const double dLN256 = log(256.0);
    const double
        dDistFactor = pow(1.0 - fTraineeDensity, g_fogPowRampCoef),
        dRealVisDist = cg::lerp01(dDistFactor,g_totalFogVisibility, g_clearDayVisibility );
    // save vis dist
    if (fVisDistPtr)
        *fVisDistPtr = float(dRealVisDist);
    // calculate fogging exponent
    return float(dLN256 / cg::sqr(dRealVisDist));
}


//////////////////////////////////////////////////////////////////////////
static float convertTraineeDensityToExpSceneFog( float fTraineeDensity, float * fVisDistPtr )
{
    // calculate real visibility distance somehow
    static const double dLN256 = log(256.0);
    const double
        dDistFactor = pow(1.0 - fTraineeDensity, g_fogPowRampCoef),
        dRealVisDist = cg::lerp01(dDistFactor, g_totalFogVisibility, g_clearDayVisibility);
    // save vis dist
    if (fVisDistPtr)
        *fVisDistPtr = float(dRealVisDist);
    // calculate fogging exponent
    return float(dLN256 / dRealVisDist);
}


// constructor
FogLayer::FogLayer(osg::Group * pScene)
    : m_fRealVisDist(30000.0f)
    , m_realExp2Density(0.f)
    , m_fogDensity (0.f)
{
    // create fog uniform for the whole scene
    osg::StateSet * pSceneSS = pScene->getOrCreateStateSet();
    _sceneFogUniform = new osg::Uniform("SceneFogParams", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    pSceneSS->addUniform(_sceneFogUniform.get());


    // state set composing
    _buildStateSet();
    // geometry creation
    _createGeometry();
}

// set fog params
void FogLayer::setFogParams( const osg::Vec3f & vFogColor, float fFogDensity )
{
    m_fogDensity =  fFogDensity;

    // for sky
    _skyFogUniform->set(osg::Vec4f(vFogColor, fFogDensity));

    // for scene                
    m_realExp2Density = convertTraineeDensityToExp2SceneFog(fFogDensity, &m_fRealVisDist);
    // save uniform
    _sceneFogUniform->set(osg::Vec4f(vFogColor, m_realExp2Density));

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
    osg::ref_ptr<osg::Program> cFogLayerProg = creators::CreateProgram("sky").program; //new osg::Program();
    cFogLayerProg->setName("FogLayerShader");
    sset->setAttribute(cFogLayerProg.get());

    // set appropriate mix blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc();
    pBlendFunc->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    sset->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

    // create fog uniform for the sky layer
    _skyFogUniform = new osg::Uniform("SkyFogParams", osg::Vec4f(1.f, 1.f, 1.f, 0.f));
    sset->addUniform(_skyFogUniform.get());
    
    
    // enable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
    sset->setAttribute(pDepth);

    // disable cull-face just for the case
    sset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);


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
