#include "stdafx.h"
#include "shadow_map.h"
#include "creators.h"

namespace creators
{
    osg::ref_ptr<ShadowMap> GetShadowMap()
    {
        static osg::ref_ptr<ShadowMap> sm = new ShadowMap(512*8);
        return sm;
    }

}

ShadowMap::ShadowMap(int width, int height)
{
    init(width,height); 
    setNumChildrenRequiringUpdateTraversal(1);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
ShadowMap::ShadowMap(const ShadowMap& sm,const osg::CopyOp& copyop)
    : osg::Group(sm,copyop)
    , _camera(sm._camera)/*osg::CameraNode(sm,copyop)*/
    , _texture(sm._texture)
{
       setNumChildrenRequiringUpdateTraversal(1);
}

#if 0
void ShadowMap::traverse(osg::NodeVisitor& nv)
{
    // So do not allow any Visitors except CULL_VISITOR
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
        osg::Group::traverse(nv);
}
#else
void ShadowMap::traverse(osg::NodeVisitor& nv)
{
    //if (_shadowTechnique.valid())
    {
        //void ShadowTechnique::traverse(osg::NodeVisitor& nv)
        {
            //if (!_shadowedScene) return;

            if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
            {
                // if (_dirty) init();

                //void ShadowMap::update(osg::NodeVisitor& nv)
                {
                    /*_shadowedScene->*/osg::Group::traverse(nv);
                }
            }
            else if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
            {
                osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
                if (cv) cull(*cv);
                else /*_shadowedScene->*/osg::Group::traverse(nv);
            }
            else
            {
                /*_shadowedScene->*/osg::Group::traverse(nv);
            }
        }
    }
    //else
    //{
    //    osg::Group::traverse(nv);
    //}
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

    _camera = new osg::Camera;
    _camera->setViewport( 0, 0, width, height );
    //_camera->setClearColor( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
    //_camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    //setCullingMode(osg::CullSettings::NO_CULLING);
    _camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    //_camera->setClearDepth(0.0f);
    //_camera->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::GEQUAL, 1.0, 0.0, true), osg::StateAttribute::OVERRIDE);
    
    _camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    _camera->setRenderOrder( osg::Camera::PRE_RENDER );
    _camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    _camera->attach( osg::Camera::DEPTH_BUFFER, _texture.get() );

    _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    _camera->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
    cf->setMode(osg::CullFace::FRONT);
    _camera->getStateSet()->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    
    _camera->setCullCallback(utils::makeNodeCallback(this, [this](osg::NodeVisitor * pNV)->void {this->osg::Group::traverse(*pNV);},true));
    
}

void ShadowMap::cull( osgUtil::CullVisitor & cv )
{
     //osgUtil::CullVisitor& cv = dynamic_cast<osgUtil::CullVisitor &>(*pNV);
     
     if(_get_light)
     {

         const osg::Light* selectLight = _get_light();

         osg::Vec4 lightpos = selectLight->getPosition();
         osg::Vec3 lightDir = selectLight->getDirection();

#if 0
         osg::Matrix eyeToWorld;
         eyeToWorld.invert(*cv.getModelViewMatrix());

         lightpos = lightpos * eyeToWorld;     
         lightDir = osg::Matrix::transform3x3( lightDir, eyeToWorld );
#endif
         lightDir.normalize();

         float fov = selectLight->getSpotCutoff() * 2;
         if(fov < 180.0f)   // spotlight, then we don't need the bounding box
         {
             osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());
             _camera->setProjectionMatrixAsPerspective(fov, 1.0, 0.1, 1000.0);
             _camera->setViewMatrixAsLookAt(position,position+lightDir,computeOrthogonalVector(lightDir));
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
             const osg::BoundingBox bb =  _bb;
             // const osg::BoundingSphere bb = _bs;
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

                 _camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
                 _camera->setViewMatrixAsLookAt(position,bb.center(),computeOrthogonalVector(bb.center()-position));
             }
             else    // directional light
             {
                 // make an orthographic projection
                 osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
                 lightDir.normalize();

                 const double radius =  bb.radius() /** 0.05*/;

                 // set the position far away along the light direction
                 osg::Vec3 position = bb.center() + lightDir * radius * 2;

                 float centerDistance = (position-bb.center()).length();

                 double znear = centerDistance-radius;
                 double zfar  = centerDistance+radius;
                 double zNearRatio = 0.001f;
                 if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                 double top   = radius;
                 double right = top;

                 //double gtop,gright,gleft,gbottom;
                 //getProjectionMatrixAsOrtho(gleft,gright,gbottom,gtop,znear,zfar);
                 osg::Vec3f eye;
                 osg::Vec3f center;
                 osg::Vec3f up;
                 float lookDistance=1.0f;
                 _camera->getViewMatrixAsLookAt(eye,center,up,lookDistance);
                 
                 //position.normalize();
                 //osg::Vec3 center = bb.center();
                 //center.normalize();
                 //osg::Vec3 up = computeOrthogonalVector(lightDir);
                 //up.normalize();

                 _camera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
                 _camera->setViewMatrixAsLookAt(position,bb.center(),/*osg::Vec3(0.0f, 0.0f, 1.0f)*/computeOrthogonalVector(lightDir));
                 //_camera->setViewMatrixAsLookAt(position,center,up); 

                 const osg::Matrix shadowBias = osg::Matrix( 0.5f,0.0f,0.0f,0.0f,
                     0.0f,0.5f,0.0f,0.0f,
                     0.0f,0.0f,0.5f,0.0f,
                     0.5f,0.5f,0.5f,1.0f
                     );

                 osg::Matrix lightPV = shadowBias * _camera->getProjectionMatrix() * _camera->getViewMatrix();

                 _shadowMat->set( osg::Matrix() /*lightPV*/ );

             }


         }

         //cv.setTraversalMask( traversalMask & 
         //    getShadowedScene()->getCastsShadowTraversalMask() );

         // do RTT camera traversal
         _camera->accept(cv);

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

void ShadowMap::setScene(osg::Node* scene )
{
    // get the bounds of the model.    
    osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    // cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());
    
    scene->asGroup()->osg::Group::traverse(cbbv);

    _bb = cbbv.getBoundingBox();
    _bs = scene->getBound();

    addChild(scene);

    osg::StateSet * pSceneSS = scene->getOrCreateStateSet();
    _shadowMat = new osg::Uniform("shadow0_matrix",osg::Matrixf());
    pSceneSS->addUniform(_shadowMat.get());
}


