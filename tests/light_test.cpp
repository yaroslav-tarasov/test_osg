#include "stdafx.h"
#include "visitors/info_visitor.h"
#include "visitors/find_tex_visitor.h"
#include "creators.h"
#include "av/avTerrain/Terrain.h"

namespace
{
	const size_t size_x = 50;//1980;
	const size_t size_z = 50;//1200;

    void createDAIGeometry( osg::Geometry& geom, int nInstances=1 )
    {
        const float halfDimX( size_x / 2.0 );
        const float halfDimZ( size_z / 2.0);

        osg::Vec3Array* v = new osg::Vec3Array;
        v->resize( 4 );
        geom.setVertexArray( v );

        // Geometry for a single quad.

        (*v)[ 0 ] = osg::Vec3( -halfDimX, 0.,  -halfDimX );
        (*v)[ 1 ] = osg::Vec3( halfDimX, 0., -halfDimX );
        (*v)[ 2 ] = osg::Vec3( halfDimX, 0., halfDimZ );
        (*v)[ 3 ] = osg::Vec3( -halfDimX, 0., halfDimZ);
#if 0
		(*v)[ 0 ] = osg::Vec3(  0., 0., 0. );
		(*v)[ 1 ] = osg::Vec3( halfDimX*2.0f, 0., 0. );
		(*v)[ 2 ] = osg::Vec3( halfDimX*2.0f, 0., halfDimZ*2.0f );
		(*v)[ 3 ] = osg::Vec3( 0., 0., halfDimZ*2.0f );
#endif
        // create color array data (each corner of our triangle will have one color component)
        osg::Vec4Array* pColors = new osg::Vec4Array;
        pColors->push_back( osg::Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        pColors->push_back( osg::Vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
        pColors->push_back( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
        pColors->push_back( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
        geom.setColorArray( pColors );

        // make sure that our geometry is using one color per vertex
        geom.setColorBinding( osg::Geometry::BIND_PER_VERTEX );

        osg::Vec2Array* pTexCoords = new osg::Vec2Array( 4 );
        (*pTexCoords)[0].set( 0.0f, 0.0f );
        (*pTexCoords)[1].set( 1.0f, 0.0f );
        (*pTexCoords)[2].set( 1.0f, 1.0f );
        (*pTexCoords)[3].set( 0.0f, 1.0f );
        geom.setTexCoordArray( 0, pTexCoords );

		osg::Vec3 normal = osg::Vec3( 0.0f, 0.0f, 1.0f);
		normal.normalize();
		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(normal);

		geom.setNormalArray(normals, osg::Array::BIND_OVERALL);
        // Use the DrawArraysInstanced PrimitiveSet and tell it to draw 1024 instances.
        geom.addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4, nInstances ) );
    }


    osg::Geometry* createGeometry()
    {
        osg::Geometry* geom = new osg::Geometry;
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        createDAIGeometry( *geom, 1*1 );

        return geom;
    }


	class UpdateCallback  : public osg::NodeCallback
	{

	public:
		UpdateCallback( )
		{}

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
		{	
			traverse(node,nv);
			
			osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
			avAssert(pCV);

			const osg::Matrixd mWorldToView = *pCV->getModelViewMatrix();

			osg::Matrix trMatrix = node->asTransform()->asMatrixTransform()->getMatrix();
		}


		osg::ref_ptr<osg::Uniform> uworldToLight;
	    osg::ref_ptr<osg::Uniform> ulightToWorld;

	};


}
namespace {
    char vertexShaderSource[] = 
        "#version 430 compatibility \n"
        "#extension GL_ARB_gpu_shader5 : enable \n"
        "out vec2 uv;  \n"
        "void main(void) \n"
        "{ \n"
        "    uv        = gl_MultiTexCoord0.xy ;\n"
        "    gl_Position        = gl_ModelViewProjectionMatrix *gl_Vertex ;\n"
        "}\n";


    char fragmentShaderSource[] =  
        "#version 430 compatibility \n"
        "#extension GL_ARB_gpu_shader5 : enable \n"
        "uniform sampler2D colorTex; \n"
        "in vec2 uv;  \n"
        "out vec4 FragColor;   \n"
        "\n"
        "void main(void) \n"
        "{ \n"
        "\n"
        "    FragColor = vec4(texture2D(colorTex, uv).xyz, 0.4);"
        "}\n";

    osg::Program* createProgram( const std::string& name, const std::string& vertexSource, const std::string& fragmentSource  )
    {
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->setName( name );

        osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);
        vertexShader->setName( name + "_vertex" );
        program->addShader(vertexShader.get());

        osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);
        fragmentShader->setName( name + "_fragment" );
        program->addShader(fragmentShader.get());

        return program.release();
    }


}

osg::Camera* createHUD()
{
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);



#if 1
    // add to this camera a subgraph to render
    {

        osg::Geode* geode = new osg::Geode();

        std::string timesFont("fonts/arial.ttf");

        //
        // create state set
        //

        // turn lighting off for the text and disable depth test to ensure it's always ontop.
        osg::StateSet* pCurStateSet = geode->getOrCreateStateSet();
        pCurStateSet->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::ref_ptr<osg::Program> program;
        program = createProgram("Logo",vertexShaderSource,fragmentShaderSource);
        pCurStateSet->setAttributeAndModes(program);	

#if 0
        pCurStateSet->addUniform(new osg::Uniform("colorTex", BASE_COLOR_TEXTURE_UNIT));
        osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
        pCurStateSet->setTextureAttributeAndModes( BASE_COLOR_TEXTURE_UNIT, avCore::GetDatabase()->LoadTexture("images/logo.dds", osg::Texture::REPEAT), value );
#endif


        //osg::Vec3 position(150.0f,800.0f,0.0f);
        osg::Vec3 position(0.0f,1000.0f,0.0f);
        //osg::Vec3 delta(0.0f,-120.0f,0.0f);
        osg::Vec3 delta(0.0f,-120.0f,0.0f);

#if 0
        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Head Up Displays are simple :-)");

            position += delta;
        }


        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("All you need to do is create your text in a subgraph.");

            position += delta;
        }


        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Then place an osg::Camera above the subgraph\n"
                "to create an orthographic projection.\n");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Set the Camera's ReferenceFrame to ABSOLUTE_RF to ensure\n"
                "it remains independent from any external model view matrices.");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("And set the Camera's clear mask to just clear the depth buffer.");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("And finally set the Camera's RenderOrder to POST_RENDER\n"
                "to make sure it's drawn last.");

            position += delta;
        }
#endif


        {
            osg::BoundingBox bb(osg::Vec3f(0,0,0),osg::Vec3f(1280,1024,0));
            //for(unsigned int i=0;i<geode->getNumDrawables();++i)
            //{
            //	bb.expandBy(geode->getDrawable(i)->getBound());
            //}


            osg::Geometry* geom = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            float depth = bb.zMin()-0.1;
            vertices->push_back(osg::Vec3(bb.xMin(),bb.yMax(),depth));
            vertices->push_back(osg::Vec3(bb.xMin(),bb.yMin(),depth));
            vertices->push_back(osg::Vec3(bb.xMax(),bb.yMin(),depth));
            vertices->push_back(osg::Vec3(bb.xMax(),bb.yMax(),depth));
            geom->setVertexArray(vertices);

            osg::Vec3Array* normals = new osg::Vec3Array;
            normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
            geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

            osg::Vec4Array* colors = new osg::Vec4Array;
            colors->push_back(osg::Vec4(1.0f,0.0,0.2f,0.2f));
            geom->setColorArray(colors, osg::Array::BIND_OVERALL);

            osg::Vec2Array* uv = new osg::Vec2Array;
            uv->push_back(osg::Vec2(0.0,0.0));
            uv->push_back(osg::Vec2(0.0,1.0));
            uv->push_back(osg::Vec2(1.0,1.0));
            uv->push_back(osg::Vec2(1.0,0.0));
            uv->setNormalize(true);
            geom->setTexCoordArray(BASE_COLOR_TEXTURE_UNIT, uv, osg::Array::BIND_PER_VERTEX);


            geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

            osg::StateSet* stateset = geom->getOrCreateStateSet();
            stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
            //stateset->setAttribute(new osg::PolygonOffset(1.0f,1.0f),osg::StateAttribute::ON);
            stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

            geode->addDrawable(geom);
        }

        camera->addChild(geode);
    }
#endif
     
    //osg::Geode* geode = new osg::Geode();
    //geode->addDrawable(createGeometry());
    //camera->addChild(geode);


    return camera;
}


bool Create( osgViewer::Viewer* vw )
{
    osg::ref_ptr<osg::Node> scene = new osg::Node;

    // create a HUD as slave camera attached to the master view.

    // vw->setUpViewAcrossAllScreens();

    osgViewer::Viewer::Windows windows;
    vw->getWindows(windows);

    if (windows.empty()) return 1;

    osg::Camera* hudCamera = createHUD();

    // set up cameras to render on the first window available.
    hudCamera->setGraphicsContext(windows[0]);
    hudCamera->setViewport(0,0,windows[0]->getTraits()->width, windows[0]->getTraits()->height);

    vw->addSlave(hudCamera, false);

    // set the scene to render
    // vw->setSceneData(scene.get());

    //if (!vw->getCameraManipulator() && vw->getCamera()->getAllowEventFocus())
    //{
    //    vw->setCameraManipulator(new osgGA::TrackballManipulator());
    //}

    return true;
}

int main_light_test( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);
	viewer.getCamera()->setClearColor(osg::Vec4(0,0,0,0));

    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");   

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;



    osg::Geode*		geodeLit = new osg::Geode();	
    osg::ref_ptr<osg::Geometry> geom = createGeometry();

    geodeLit->setCullingActive( false );   
    geodeLit->addDrawable( geom.get() );
    
	osg::StateSet * pStateSet = geodeLit->getOrCreateStateSet();
	osg::ref_ptr<osg::PositionAttitudeTransform> mt = new osg::PositionAttitudeTransform;
	mt->setPosition(osg::Vec3(0,14.5,1));
	mt->addChild(geodeLit);
    
	root->addChild(mt);
	// root->setNodeCallback(new UpdateCallback);


	const std::string name = "a_319";
	osg::Node* model_file = osgDB::readNodeFile("data/models/" + name + "/" + name + ".dae");
    root->addChild(model_file);
	model_file->getOrCreateStateSet()->setRenderBinDetails(2, "DepthSortedBin");

	osg::Matrix m;
	osg::Matrix mi;

	m.makeTranslate(osg::Vec3(10000,0,50));
	pStateSet->addUniform(new osg::Uniform("g_lightToWorld"  , m));

	mi = osg::Matrix::inverse(m);
	pStateSet->addUniform(new osg::Uniform("g_worldToLight"           , mi));
	pStateSet->addUniform(new osg::Uniform("g_lightCol"               , osg::Vec4(1,1,1,0.87)));
	pStateSet->addUniform(new osg::Uniform("g_scatteringCoefficient"  , 1.f));


    osg::ref_ptr<osg::Program> cLightProg = creators::createProgram("splight").program; 
    cLightProg->setName("LightShader");
    pStateSet->setAttributeAndModes(cLightProg.get());
	pStateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
	
	pStateSet->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	//pStateSet->setAttribute( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.99f), osg::StateAttribute::ON );

	// setup blending
	osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	pStateSet->setAttributeAndModes(pBlendFunc, osg::StateAttribute::ON);

	osg::BlendEquation* pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
	pStateSet->setAttributeAndModes(pBlendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
	
	//pStateSet->setRenderBinDetails(0, "DepthSortedBin");
	//pStateSet->setRenderBinDetails(1, "DepthSortedBin");
	//pStateSet->setNestRenderBins(false);

	viewer.apply(new osgViewer::SingleScreen(1));

    Create(&viewer);

    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}

AUTO_REG(main_light_test)