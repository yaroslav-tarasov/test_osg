#include "stdafx.h"
#include "Prerender.h"
#include "materials.h"

Prerender::Prerender()
{
    init();
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


void Prerender::init()
{
    _texture = creators::getTextureHolder().getLightMapTexture();

    setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    setClearDepth(1.0);
    
    setClearColor(osg::Vec4(0.0,0.0,0.0,0.0));
    setColorMask(true, true, true, true);

    // just inherit the main cameras view
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    setProjectionMatrix(osg::Matrixd::identity());
    setViewMatrix(osg::Matrixd::identity());

    // set viewport
    setViewport(0,0,_texture->getTextureWidth(),_texture->getTextureHeight());

    // set the camera to render before the main camera.
    setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);
    setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    setRenderOrder(osg::CameraNode::PRE_RENDER);

    // attach the texture and use it as the color buffer.
    attach(osg::CameraNode::COLOR_BUFFER, _texture.get(), 0, 0, true, 0, 0);

}

