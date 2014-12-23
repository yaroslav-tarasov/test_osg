#include "stdafx.h"
#include "creators.h"

namespace avEnv
{

class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
public:

    typedef std::vector< osg::ref_ptr<osg::Camera> >  CameraList;

    UpdateCameraAndTexGenCallback(osg::NodePath& reflectorNodePath, CameraList& Cameras)
        :  _reflectorNodePath(reflectorNodePath)
        ,  _Cameras(Cameras)
    {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        // first update subgraph to make sure objects are all moved into position
        traverse(node,nv);

        // compute the position of the center of the reflector subgraph
        osg::Matrixd worldToLocal = osg::computeWorldToLocal(nv->getNodePath()/*_reflectorNodePath*/);
        osg::BoundingSphere bs = nv->getNodePath().back()->getBound(); //_reflectorNodePath.back()->getBound();
        osg::Vec3 position = bs.center();

        typedef std::pair<osg::Vec3, osg::Vec3> ImageData;
        const ImageData id[] =
        {
            ImageData( osg::Vec3( 1,  0,  0), osg::Vec3( 0, -1,  0) ), // +X
            ImageData( osg::Vec3(-1,  0,  0), osg::Vec3( 0, -1,  0) ), // -X
            ImageData( osg::Vec3( 0,  1,  0), osg::Vec3( 0,  0,  1) ), // +Y
            ImageData( osg::Vec3( 0, -1,  0), osg::Vec3( 0,  0, -1) ), // -Y
            ImageData( osg::Vec3( 0,  0,  1), osg::Vec3( 0, -1,  0) ), // +Z
            ImageData( osg::Vec3( 0,  0, -1), osg::Vec3( 0, -1,  0) )  // -Z

        };

        for(unsigned int i=0; 
            i<6 && i<_Cameras.size();
            ++i)
        {
            osg::Matrix localOffset;
            localOffset.makeLookAt(position,position+id[i].first,id[i].second);

            osg::Matrix viewMatrix = worldToLocal*localOffset;

            _Cameras[i]->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            _Cameras[i]->setProjectionMatrixAsFrustum(-1.0,1.0,-1.0,1.0,1.0,10/*10000.0*/);
            _Cameras[i]->setViewMatrix(viewMatrix);
        }
    }

protected:

    virtual ~UpdateCameraAndTexGenCallback() {}

    osg::NodePath               _reflectorNodePath;        
    CameraList                  _Cameras;
};

osg::Group* createPrerender(osg::Node* reflectedSubgraph, osg::NodePath reflectorNodePath, unsigned int unit, const osg::Vec4& clearColor,  osg::Camera::RenderTargetImplementation renderImplementation)
{

    osg::Group* group = new osg::Group;

    osg::TextureCubeMap* texture = creators::GetTextureHolder().GetEnvTexture(); 
    unsigned tex_width = texture->getTextureWidth();
    unsigned tex_height = texture->getTextureHeight();

    // set up the render to texture cameras.
    UpdateCameraAndTexGenCallback::CameraList Cameras;
    for(unsigned int i=0; i<6; ++i)
    {
        // create the camera
        osg::Camera* camera = new osg::Camera;

        camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //camera->setClearColor(clearColor);

        // set viewport
        camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, i);

        // add subgraph to render
        camera->addChild(reflectedSubgraph);

        camera->setCullMask(~NODE_STARFIELD_MASK);  // We don't need stars on the mountains

        group->addChild(camera);

        Cameras.push_back(camera);
    }

#if 0
    // create the texgen node to project the tex coords onto the subgraph
    osg::TexGenNode* texgenNode = new osg::TexGenNode;
    texgenNode->getTexGen()->setMode(osg::TexGen::REFLECTION_MAP);
    texgenNode->setTextureUnit(unit);
    group->addChild(texgenNode);

    // set the reflected subgraph so that it uses the texture and tex gen settings.    
    {
        osg::Node* reflectorNode = reflectorNodePath.front();
        group->addChild(reflectorNode);

        osg::StateSet* stateset = reflectorNode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(unit,texture,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

        osg::TexMat* texmat = new osg::TexMat;
        stateset->setTextureAttributeAndModes(unit,texmat,osg::StateAttribute::ON);

        reflectorNode->setCullCallback(new TexMatCullCallback(texmat));
    }
#endif

    // add the reflector scene to draw just as normal
    group->addChild(reflectedSubgraph);

    // set an update callback to keep moving the camera and tex gen in the right direction.
    group->setUpdateCallback(new UpdateCameraAndTexGenCallback(reflectorNodePath, Cameras));

    return group;
}

}