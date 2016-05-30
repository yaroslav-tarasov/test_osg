#include "stdafx.h"
#include "av/precompiled.h"

#include "Prerender.h"
#include "materials.h"

namespace avCore
{

Prerender::Prerender()
    : _on (true)
{
    init();
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
Prerender::Prerender(const Prerender& prerender,const osg::CopyOp& copyop):
    osg::Camera(prerender,copyop),
    _texture(prerender._texture)
{
}

void Prerender::traverse(osg::NodeVisitor& nv)
{
    if (!_on)
        return;

    // So do not allow any Visitors except CULL_VISITOR
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
         osg::Group::traverse(nv);
}


void Prerender::init()
{
    _texture = creators::getTextureHolder().getReflTexture();

    // set clear the color and depth buffer
    setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    setClearColor(osg::Vec4(0,0,0,0));
    setClearDepth(1.0);
    setColorMask(true, true, true, true);

    // just inherit the main cameras view
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    setProjectionMatrix(osg::Matrixd::identity());
    setViewMatrix(osg::Matrixd::scale(1,1,-1)); // Flip Z axis

    // set viewport
    setViewport(0,0,_texture->getTextureWidth(),_texture->getTextureHeight());

    // set the camera to render before the main camera.
    setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    setRenderOrder(osg::Camera::PRE_RENDER);

    // attach the texture and use it as the color buffer.
    attach(osg::Camera::COLOR_BUFFER, _texture.get(), 0, 0, false, 0, 0);

    // swap frontface order
    getOrCreateStateSet()->setAttribute(new osg::FrontFace(osg::FrontFace::CLOCKWISE));

#if 1
    int im = getInheritanceMask();
    im &= ~(osg::CullSettings::CULL_MASK);
    setInheritanceMask(im);
    setCullMask(REFLECTION_MASK);
#endif

}

}