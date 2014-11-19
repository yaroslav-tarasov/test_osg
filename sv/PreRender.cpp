#include "stdafx.h"
#include "Prerender.h"


Prerender::Prerender(int width, int height)
{
    init(width,height);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
Prerender::Prerender(const Prerender& prerender,const osg::CopyOp& copyop):
    osg::CameraNode(prerender,copyop),
    _texture(prerender._texture)
{
}

void Prerender::traverse(osg::NodeVisitor& nv)
{
    // So do not allow any Visitors except CULL_VISITOR
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
        osg::Group::traverse(nv);
}

osg::Texture2D * Prerender::createTexture(int width, int height)
{
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(width,height);
    texture->setInternalFormat(GL_RGBA);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setUseHardwareMipMapGeneration(false);
    return texture;
}

void Prerender::init(int width, int height)
{
    _texture = createTexture(width,height);

    // set clear the color and depth buffer
    setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    setClearColor(osg::Vec4(0,0,0,0));
    setClearDepth(1.0);

    // just inherit the main cameras view
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    setProjectionMatrix(osg::Matrixd::identity());
    setViewMatrix(osg::Matrixd::scale(1,1,-1)); // Flip Z axis

    // set viewport
    setViewport(0,0,width,height);

    // set the camera to render before the main camera.
    setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);
    setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    setRenderOrder(osg::CameraNode::PRE_RENDER);

    // attach the texture and use it as the color buffer.
    attach(osg::CameraNode::COLOR_BUFFER, _texture.get(), 0, 0, false, 0, 0);

    // swap frontface order
    getOrCreateStateSet()->setAttribute(new osg::FrontFace(osg::FrontFace::CLOCKWISE));

    // don't reflect stars,ships,area (if nodemask is set)
    #define REFLECTION_CULL_MASK 0x00010000

    int im = getInheritanceMask();
    im &= ~(osg::CullSettings::CULL_MASK);
    setInheritanceMask(im);
    setCullMask(REFLECTION_CULL_MASK);
}

