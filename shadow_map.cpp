#include "stdafx.h"
#include "shadow_map.h"
#include "creators.h"

ShadowMap::ShadowMap(int width, int height)
{
    init(width,height);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
ShadowMap::ShadowMap(const ShadowMap& sm,const osg::CopyOp& copyop)
    :osg::CameraNode(sm,copyop)
    ,_texture(sm._texture)
{
}

#if 0
void ShadowMap::traverse(osg::NodeVisitor& nv)
{
    // So do not allow any Visitors except CULL_VISITOR
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
        osg::Group::traverse(nv);
}
#endif

osg::Texture2D * ShadowMap::createTexture(int width, int height)
{
    osg::ref_ptr<osg::Texture2D> textureFBO = new osg::Texture2D;
    textureFBO->setTextureSize( width, height );

    textureFBO->setSourceFormat(GL_DEPTH_COMPONENT); 
    textureFBO->setInternalFormat(GL_DEPTH_COMPONENT32); 
    textureFBO->setSourceType(GL_UNSIGNED_INT); 

    //textureFBO->setInternalFormat(GL_DEPTH_COMPONENT); 

    textureFBO->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR/*NEAREST*/ );
    textureFBO->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR/*NEAREST*/ );
    textureFBO->setBorderColor(osg::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
    textureFBO->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
    textureFBO->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
    // NB GL_COMPARE_REF_TO_TEXTURE è GL_COMPARE_R_TO_TEXTURE are equal
    textureFBO->setShadowComparison(true);            // Sets GL_TEXTURE_COMPARE_MODE_ARB to GL_COMPARE_R_TO_TEXTURE_ARB
    textureFBO->setShadowCompareFunc(osg::Texture::LESS);

    return textureFBO.release();
}

void ShadowMap::init(int width, int height)
{
    _texture = createTexture(width,height);

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    setViewport( 0, 0, width, height );
    //setClearColor( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
    //setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    //setCullingMode(osg::CullSettings::NO_CULLING);
    setClearMask(GL_DEPTH_BUFFER_BIT);
    //setClearDepth(0.0f);
    //getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::GEQUAL, 1.0, 0.0, true), osg::StateAttribute::OVERRIDE);
    
    setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    setRenderOrder( osg::Camera::PRE_RENDER );
    setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    attach( osg::Camera::DEPTH_BUFFER, _texture.get() );

    setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
    cf->setMode(osg::CullFace::FRONT);
    getStateSet()->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    
    setCullCallback(utils::makeNodeCallback(this, &ShadowMap::cull));
}

void ShadowMap::cull( osg::NodeVisitor * pNV )
{
     osgUtil::CullVisitor& cv = dynamic_cast<osgUtil::CullVisitor &>(*pNV);
     
     if(_get_light)
     {

         const osg::Light* selectLight = _get_light();

         osg::Vec4 lightpos = selectLight->getPosition();
         osg::Vec3 lightDir = selectLight->getDirection();

         osg::Matrix eyeToWorld;
         eyeToWorld.invert(*cv.getModelViewMatrix());

         lightpos = lightpos * eyeToWorld;     
         lightDir = osg::Matrix::transform3x3( lightDir, eyeToWorld );
         lightDir.normalize();

         float fov = selectLight->getSpotCutoff() * 2;
         if(fov < 180.0f)   // spotlight, then we don't need the bounding box
         {
             osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());
             /*_camera->*/setProjectionMatrixAsPerspective(fov, 1.0, 0.1, 1000.0);
             /*_camera->*/setViewMatrixAsLookAt(position,position+lightDir,computeOrthogonalVector(lightDir));
         }
         else
         {
#if 0
             // get the bounds of the model.    
             osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
             // cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());
             auto _shadowedScene = getChild(0)->asGroup();

             _shadowedScene->osg::Group::traverse(cbbv);

             osg::BoundingBox bb = cbbv.getBoundingBox();
#else
             // osg::BoundingBox bb =  _bb;
             osg::BoundingSphere bb = _bs;
#endif

             if (lightpos[3]!=0.0)   // point light
             {
                 osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());

                 float centerDistance = (position-bb.center()).length();

                 float znear = centerDistance-bb.radius();
                 float zfar  = centerDistance+bb.radius();
                 float zNearRatio = 0.001f;
                 if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                 float top   = (bb.radius()/centerDistance)*znear;
                 float right = top;

                 /*_camera->*/setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
                 /*_camera->*/setViewMatrixAsLookAt(position,bb.center(),computeOrthogonalVector(bb.center()-position));
             }
             else    // directional light
             {
                 // make an orthographic projection
                 // osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
                 lightDir.normalize();

                 // set the position far away along the light direction
                 osg::Vec3 position = bb.center() + lightDir * bb.radius() / 10; //+ lightDir * bb.radius() * 2;

                 float centerDistance = (position-bb.center()).length();

                 double znear = centerDistance-bb.radius();
                 double zfar  = centerDistance+bb.radius();
                 double zNearRatio = 0.001f;
                 if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                 double top   = bb.radius();
                 double right = top;


                 //double gtop,gright,gleft,gbottom;
                 //getProjectionMatrixAsOrtho(gleft,gright,gbottom,gtop,znear,zfar);
                 //osg::Vec3f eye;
                 //osg::Vec3f center;
                 //osg::Vec3f up;
                 //float lookDistance=1.0f;
                 //getViewMatrixAsLookAt(eye,center,up,lookDistance);
                 
                 osg::Vec3 center = bb.center();
                 center.normalize();
                 position.normalize();
                 osg::Vec3 up = computeOrthogonalVector(lightDir);
                 up.normalize();

                 //setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
                 //setViewMatrixAsLookAt(position,bb.center(),computeOrthogonalVector(lightDir));
                 setViewMatrixAsLookAt(position,center,up); 
             }


         }

         //cv.setTraversalMask( traversalMask & 
         //    getShadowedScene()->getCastsShadowTraversalMask() );

         // do RTT camera traversal
        // /* _camera->*/accept(cv);

     }
}

osg::Vec3 ShadowMap::computeOrthogonalVector(const osg::Vec3& direction) const
{
    float length = direction.length();
    osg::Vec3 orthogonalVector = direction ^ osg::Vec3(0.0f, 1.0f, 0.0f);
    if (orthogonalVector.normalize()<length*0.5f)
    {
        orthogonalVector = direction ^ osg::Vec3(0.0f, 0.0f, 1.0f);
        orthogonalVector.normalize();
    }
    return orthogonalVector;
}

void ShadowMap::setScene(osg::Group* scene )
{
    // get the bounds of the model.    
    osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    // cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());
    
    scene->asGroup()->osg::Group::traverse(cbbv);

    _bb = cbbv.getBoundingBox();
    _bs = scene->getBound();

    addChild(scene);
}