#include "stdafx.h"
#include "find_node_visitor.h" 
#include "info_visitor.h"
#include "find_tex_visitor.h"
#include "creators.h"
#include "sv/FogLayer.h"
#include "sv/Prerender.h"

osg::Image* createSpotLight( const osg::Vec4& centerColor, const osg::Vec4& bgColor, unsigned int size, float power )
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( size, size, 1, GL_RGBA, GL_UNSIGNED_BYTE );

    float mid = (float(size)-1) * 0.5f;
    float div = 2.0f / float(size);
    for( unsigned int r=0; r<size; ++r )
    {
        unsigned char* ptr = image->data(0, r, 0);
        for( unsigned int c=0; c<size; ++c )
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float r = powf(1.0f - sqrtf(dx*dx+dy*dy), power);
            if ( r<0.0f ) r = 0.0f;

            osg::Vec4 color = centerColor*r + bgColor*(1.0f - r);
            *ptr++ = (unsigned char)((color[0]) * 255.0f);
            *ptr++ = (unsigned char)((color[1]) * 255.0f);
            *ptr++ = (unsigned char)((color[2]) * 255.0f);
            *ptr++ = (unsigned char)((color[3]) * 255.0f);
        }
    }
    return image.release();
}


class TexChangeHandler : public osgGA::GUIEventHandler
{
public:
    TexChangeHandler(osg::Node * root,osg::Texture *texture) 
      :_texture    (texture)
      ,_root       (root)
      {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {            
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add || ea.getKey()== osgGA::GUIEventAdapter::KEY_KP_Subtract)
            { 
                if ( _root )
                { 
                    ReplaceTextureVisitor ftv( _texture.get() );
                    _root->accept( ftv );
                    _texture =  ftv.getOldTexture();
                }
                return true;
            }
        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Change texture");
        usage.addKeyboardMouseBinding("Keypad -",       "Change texture");

    }

    osg::ref_ptr<osg::Texture> _texture;
    osg::ref_ptr<osg::Node> _root;
};


int main_tex_test( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    //root->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),1.0f,0.0f,0.0f));
#if 0
    osg::Vec4 centerColor( 1.0f, 1.0f, 0.0f, 1.0f );
    osg::Vec4 bgColor( 0.0f, 0.0f, 0.0f, 1.0f );

    osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 3.0f) );
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 3.5f) );
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 4.0f) );
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 3.5f) );

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( imageSequence.get() );
#endif
    
    osg::ref_ptr<osg::Node> sub_model = osgDB::readNodeFile( "glider.osg" );

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(osgDB::readImageFile("a_319_aeroflot.png"/*"Fieldstone.jpg"*/));

    creators::nodes_array_t plane = creators::loadAirplaneParts("a_319");
    auto airplane = plane[1];

    // osg::Node*  airplane = // creators::loadAirplane();
    
    //InfoVisitor infoVisitor;
    //airplane->accept( infoVisitor );

    //findNodeVisitor findNode("Body_",findNodeVisitor::not_exact); 
    //airplane->accept(findNode);
    //auto a_node =  findNode.getFirst();
    //
    //findNodeByType<osg::Geode> findGeode; 
    //a_node->accept(findGeode);
    //auto g_node =  findGeode.getFirst();
    
    //auto dr = g_node->asGeode()->getDrawable(0);
    //
    //auto dr_num =  g_node->asGeode()->getNumDrawables();

    // g_node->setNodeMask(0);

    //osg::StateSet* stateset = g_node->asGeode()->getDrawable(0)->getOrCreateStateSet();
    // А вот тут у нас координаты не пойми чего
    // osg::Array* a = g_node->asGeode()->getDrawable(0)->asGeometry()->getTexCoordArray(1);

    //stateset->setTextureAttributeAndModes( 0, /*texture.get()*/new osg::Texture2D());
    //g_node->setStateSet(stateset);
    
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum( 0 );
    light->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    light->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    light->setPosition( osg::Vec4(100.0f, 10.0f, 10.0f, 1.0f) );

    osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
    source->setLight( light ); 

#if 0
    osg::ref_ptr<osgFX::BumpMapping> effet = new osgFX::BumpMapping();
    effet->setLightNumber(0);
    effet->setOverrideDiffuseTexture(texture.get());
    effet->setOverrideNormalMapTexture(new osg::Texture2D(osgDB::readImageFile("a_319_n.png")));
    effet->addChild(airplane);
    // effet->prepareChildren();
    effet->setEnabled(false);
#endif

	osg::ref_ptr<avSky::FogLayer> skyFogLayer = new avSky::FogLayer(root->asGroup());
    root->addChild(skyFogLayer.get());
    skyFogLayer->setFogParams(osg::Vec3f(1.5,1.5,1.5),0.1);    // (вроде начинаем с 0.1 до максимум 1.0)
    float coeff = skyFogLayer->getFogExp2Coef();
    
    osg::StateSet * pCommonStateSet = airplane->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(/*RENDER_BIN_SOLID_MODELS*/100, "RenderBin");


#if 1
    int tex_width = 1024, tex_height = 1024;


    osg::ref_ptr<osg::Texture2D> textureFBO = new osg::Texture2D;
    textureFBO->setTextureSize( tex_width, tex_height );
    textureFBO->setInternalFormat( GL_RGBA );
    textureFBO->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    textureFBO->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    textureFBO->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
    textureFBO->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setViewport( 0, 0, tex_width, tex_height );
    camera->setClearColor( osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f) );
    camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    camera->setRenderOrder( osg::Camera::PRE_RENDER );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->attach( osg::Camera::COLOR_BUFFER, textureFBO.get() );

    const osg::BoundingSphere& bs = airplane->getBound();
    //if (!bs.valid())
    //{
    //    return subgraph;
    //}

    float znear = 1.0f*bs.radius();
    float zfar  = 3.0f*bs.radius();

    // 2:1 aspect ratio as per flag geometry below.
    float proj_top   = 0.5f*znear;
    float proj_right = 0.5f*znear;

    //znear *= 0.9f;
    //zfar *= 1.1f;

    // set up projection.
    camera->setProjectionMatrixAsFrustum(-proj_right,proj_right,-proj_top,proj_top,znear,zfar);
    //double fl,fr,bt,tp,zn,zf;
    //viewer.getCamera()->getProjectionMatrixAsFrustum(fl,fr,bt,tp,zn,zf);
    //camera->setProjectionMatrixAsFrustum(fl,fr,bt,tp,zn,zf );

    // set view
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrixAsLookAt(bs.center()-osg::Vec3(-2.0f,0.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));


    camera->addChild( airplane/*effet.get()*/ );
    camera->addChild( skyFogLayer.get()       );

    root->addChild(camera.get());
#endif
                                
    // effet->setUpDemo();
    // effet->setEnabled(false);
    root->addChild(source);
    root->addChild(airplane/*effet.get()*/);
    root->addChild(sub_model.get());

#if 0
    // add creation of main reflection texture
    static const int
        g_nReflectionWidth = 512,
        g_nReflectionHeight = 512;
    osg::ref_ptr<Prerender> pReflFBOGroup = new Prerender(g_nReflectionWidth, g_nReflectionHeight);
    // auto m_groupMainReflection = pReflFBOGroup.get();

    // tricky cull for reflections
    osg::ref_ptr<osg::Group> reflectionSubGroup = new osg::Group();
    reflectionSubGroup->addChild(pReflFBOGroup.get());
    //reflectionSubGroup->setCullCallback(new ReflectionCullCallback());
    pReflFBOGroup->addChild(airplane);
    root->addChild(reflectionSubGroup.get());
#endif

  
    // osgDB::writeNodeFile(*root,"tex_test_blank.osgt");
	
	// Set the clear color to black
    viewer.getCamera()->setClearColor(osg::Vec4(1.0,0,0,1));

    viewer.addEventHandler( new TexChangeHandler( root.get(), texture.get() ) );
	viewer.addEventHandler( new avSky::FogHandler([&](osg::Vec4f v){skyFogLayer->setFogParams(osg::Vec3f(1.0,1.0,1.0),v.w());}, osg::Vec3f(0.5,0.5,0.5) ));
    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}

AUTO_REG(main_tex_test)