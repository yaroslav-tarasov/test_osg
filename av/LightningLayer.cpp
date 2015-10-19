#include "stdafx.h"

#include "av/precompiled.h"

#include "LightningLayer.h"

#include "utils/materials.h"

//
// Global namespace
//

namespace avSky
{

// constructor
LightningLayer::LightningLayer(osg::Group * pScene)
    : _clDensity(0.f)
{
     
    // state set composing
    _buildStateSet();
    // geometry creation
    _createGeometry();
    loadTextures();
   setDensity(1.0);
}

// set clouds 2 color scheme
void LightningLayer::setColors( const osg::Vec3f & vFrontColor, const osg::Vec3f & vBackColor )
{
    _colorFrontUniform->set(osg::Vec3f(vFrontColor));
    _colorBackUniform->set(osg::Vec3f(vBackColor));
}

// set density
bool LightningLayer::setDensity( float density )
{

   if (!cg::eq(_clDensity, density, 0.01f))
   {
       _clDensity = density;
       _densityUniform->set(_clDensity);
       return true;
   }
   return false;
}

// movement
void LightningLayer::setRotationSiderealTime( float rot_deg )
{
    //_clRotation = cg::transform_4f(
    //    cg::translation_3f(),
    //    cg::rotation_3f(cg::rot_axisf(cg::point_3f(0, 0, 1), rot_deg)),
    //    cg::scale_3f());

    const osg::Quat quat0(osg::inDegrees(0.0f), osg::X_AXIS,                      
        osg::inDegrees(0.f)  , osg::Y_AXIS,
        osg::inDegrees(rot_deg)  , osg::Z_AXIS ); 

    _clRotation = osg::Matrix::rotate(quat0);
    _clRotation = osg::Matrix::rotate( osg::inDegrees(rot_deg) , osg::Vec3(0, 0, 1));
    _mvpUniform ->set(_clRotation);
}


bool LightningLayer::loadTextures()
{
    static const std::array<std::string const, clouds_types_num> tex_names = {
        "images/lightning/FlashImgBranch1.dds",
        "images/lightning/FlashImgBranch2.dds",
        "images/lightning/FlashImgBranch3.dds",
        "images/lightning/FlashL_new.dds"
    };


    osg::StateSet * sset = getOrCreateStateSet();
        
    osg::ref_ptr<osg::Texture2D> cloudsTex = new osg::Texture2D;
    cloudsTex->setImage( osgDB::readImageFile("images/lightning/FlashImgBranch2.png"/*tex_names[1]*/,new osgDB::Options("")) );
    cloudsTex->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
    cloudsTex->setMaxAnisotropy(16.0f);
    cloudsTex->setWrap(  osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER );
    cloudsTex->setWrap(  osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER );
    cloudsTex->setWrap(  osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER );

    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED;
    sset->setTextureAttributeAndModes( 1, cloudsTex.get(), value );

    return true;
}

//
// Some geometry related functions
//

// create state set
void LightningLayer::_buildStateSet()
{
    osg::StateSet * sset = getOrCreateStateSet();
    
    //sset->setMode( GL_BLEND,    osg::StateAttribute::ON     | osg::StateAttribute::PROTECTED );

    // set render bin
    sset->setRenderBinDetails(RENDER_BIN_LIGHTNING, "RenderBin");
    sset->setNestRenderBins(false);

    // create clarity shader
    osg::ref_ptr<osg::Program> cCloudsLayerProg = creators::createProgram("lightning").program; 
    cCloudsLayerProg->setName("LightningLayerShader");
    sset->setAttribute(cCloudsLayerProg.get());

    // set appropriate mix blending
    osg::BlendFunc * pBlendFunc = new osg::BlendFunc();
    pBlendFunc->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    sset->setAttributeAndModes(pBlendFunc, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    
    osg::BlendEquation* blendEquation = new osg::BlendEquation(osg::BlendEquation::RGBA_MAX/*FUNC_ADD*/);
    blendEquation->setDataVariance(osg::Object::DYNAMIC);
    sset->setAttributeAndModes(blendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

//#ifdef ORIG_EPHEMERIS      
    // enable depth test but disable depth write
    osg::Depth * pDepth = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
    sset->setAttribute(pDepth,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // disable cull-face just for the case
    sset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
//#endif

    _colorFrontUniform = new osg::Uniform("frontColor", osg::Vec3f(1.f, 1.f, 1.f));
    _colorBackUniform  = new osg::Uniform("backColor",  osg::Vec3f(1.f, 1.f, 1.f));
    _densityUniform    = new osg::Uniform("density",    _clDensity);
    _mvpUniform              = new osg::Uniform("MVP",        _clRotation);

    sset->addUniform( new osg::Uniform("Lightning", 1) );
    sset->addUniform(_colorFrontUniform.get());
    sset->addUniform(_colorBackUniform.get());
    sset->addUniform(_densityUniform.get());
    sset->addUniform(_mvpUniform.get());


    return;
}

// create fake layer geometry
void LightningLayer::_createGeometry()
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


}
