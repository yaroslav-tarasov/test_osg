#include "stdafx.h"
#include "av/precompiled.h"
#include "Sky.h"
#include "EphemerisModel.h"
#include "av/CloudLayer.h"
#include "av/EnvRenderer.h"

using namespace avSky;

//////////////////////////////////////////////////////////////////////////
Sky::Sky( osg::Group * pScene )
{
    setCullingActive(false);

    // Create Ephemeris skydome
    _cSkydomePtr = new EphemerisModel(pScene);
    // addChild(_cSkydomePtr.get());

    // Create clouds
    //_cCloudsPtr = new avSky::CloudsLayer(pScene);
    //addChild(_cCloudsPtr.get());

    // global sky state-set
    osg::StateSet * pSkyStateSet = getOrCreateStateSet();

    pSkyStateSet->setNestRenderBins(false);

    // disable back face culling
    pSkyStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // disable writing to depth buffer
    osg::Depth * pDepth = new osg::Depth;
    pDepth->setWriteMask(false);
    pSkyStateSet->setAttribute(pDepth);

    osg::ref_ptr<osg::Group> fbo_node = new osg::Group;
    fbo_node->addChild(_cSkydomePtr.get());
    // fbo_node->addChild(_cCloudsPtr.get());

    addChild(avEnv::createPrerender(fbo_node,osg::NodePath(),0,osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f),osg::Camera::FRAME_BUFFER_OBJECT));

    // setStarFieldMask(NODE_STARFIELD_MASK);  
    // setNodeMask( DO_NOT_PICK_NODE_MASK );
}

//////////////////////////////////////////////////////////////////////////
float Sky::GetSunIntensity() const
{
    return _cSkydomePtr->getIllumination();
}

//////////////////////////////////////////////////////////////////////////
const osg::Vec3f & Sky::GetFogColor() const
{
    return _cSkydomePtr->getFogColor();
}

//////////////////////////////////////////////////////////////////////////
void Sky::SetWaterColor( const osg::Vec3f & cWaterColor )
{
    _cSkydomePtr->updateUnderWaterColor(cWaterColor);
}

osg::LightSource* Sky::getSunLightSource()
{
    return _cSkydomePtr->getSunLightSource(); 
}

