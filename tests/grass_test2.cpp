#include "stdafx.h"
#include "visitors/info_visitor.h"
#include "visitors/find_tex_visitor.h"
#include "visitors/find_node_visitor.h"
#include "creators.h"
#include "av/avTerrain/Terrain.h"

namespace
{

	char vertexShaderSource[] = 
		"#version 430 compatibility \n"
		"#extension GL_ARB_gpu_shader5 : enable \n"
		"out vec4 color; \n"
		"out vec2 texcoord; \n"
		"void main(void) \n"
		"{ \n"
		"    color       = vec4(vec3(0.0),1.0);\n"
		"    texcoord    = gl_MultiTexCoord1.xy;  \n"
		"    gl_Position = gl_ProjectionMatrix  * gl_ModelViewMatrix * gl_Vertex ;\n"
		"}\n";

	// * instanceModelMatrix

	char fragmentShaderSource[] =  
		"#version 430 compatibility \n"
		"#extension GL_ARB_gpu_shader5 : enable \n"
		"in vec4 color; \n"
		"in vec2 texcoord; \n"
		"uniform sampler2D Texture1; \n"
		"uniform sampler2D Texture2; \n"
		"uniform sampler2D Texture3; \n"
		"//uniform sampler2DArray  arrTex; \n"
		"out vec4 FragColor;   \n"
		"\n"
		"void main(void) \n"
		"{ \n"
		"\n"
		"    //FragColor = texture2DArray(arrTex, vec3(texcoord.xy,inst_id==3?2:inst_id % 2)); \n"
		"    vec4 tex3 = texture2D(Texture3,texcoord.xy * vec2(16.f,24.f)); \n"
		"    vec4 tex2 = texture2D(Texture2,texcoord.xy * vec2(16.f,24.f)); \n"
		"    FragColor = mix(tex3,tex2,texture2D(Texture1,texcoord.xy).r); \n"
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

	void   _buildStateSet(osg::Node* node)
	{
		std::string grass_vert = Database::LoadShader("blender/grass/grass.vert");     

		if (grass_vert.empty())
			return;

		std::string grass_geom = Database::LoadShader("blender/grass/grass.geom");     

		if (grass_geom.empty())
			return;

		std::string grass_frag = Database::LoadShader("blender/grass/grass.frag");     

		if (grass_frag.empty())
			return;


		osg::Shader* vertexShader = new osg::Shader();
		vertexShader->setType( osg::Shader::VERTEX );
		vertexShader->setShaderSource(grass_vert);

		osg::Shader* geomShader = new osg::Shader();
		geomShader->setType( osg::Shader::GEOMETRY );
		geomShader->setShaderSource(grass_geom);

		osg::Shader* fragShader = new osg::Shader();
		fragShader->setType( osg::Shader::FRAGMENT );
		fragShader->setShaderSource(grass_frag);

		osg::Program*  program = new osg::Program();
		program->addShader( vertexShader );
		program->addShader( geomShader );
		program->addShader( fragShader );

		osg::StateSet* ss = node->getOrCreateStateSet();
		ss->setAttribute( program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );


		osg::Texture2D* pTex1 = new osg::Texture2D;
		osg::Image* pImage1 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/GrassHeight1.png") );
		pTex1->setImage( pImage1 );
		ss->setTextureAttributeAndModes( 0, pTex1, osg::StateAttribute::ON );
		ss->addUniform( new osg::Uniform("Texture1"      , 0) );

		osg::Texture2D* pTex2 = new osg::Texture2D;
		osg::Image* pImage2 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/Grass1.png") );
		pTex2->setImage( pImage2 );
		ss->setTextureAttributeAndModes( 1, pTex2, osg::StateAttribute::ON );
		ss->addUniform( new osg::Uniform("Texture2"      , 1) );
        
        pTex2->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTex2->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTex2->setUseHardwareMipMapGeneration(true);

		osg::Texture2D* pTex3 = new osg::Texture2D;
		osg::Image* pImage3 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/TallGrass.Png") );
		pTex3->setImage( pImage3 );
		ss->setTextureAttributeAndModes( 2, pTex3, osg::StateAttribute::ON );
		ss->addUniform( new osg::Uniform("Texture3"      , 2) );
        
        pTex3->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTex3->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTex3->setUseHardwareMipMapGeneration(true);

		osg::Texture2D* pTex4 = new osg::Texture2D;
		osg::Image* pImage4 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/GrassBush.Png") );
		pTex4->setImage( pImage4 );
		ss->setTextureAttributeAndModes( 3, pTex4, osg::StateAttribute::ON );
		ss->addUniform( new osg::Uniform("Texture4"      , 3) );
        
        pTex4->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTex4->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        pTex4->setUseHardwareMipMapGeneration(true);

#if 0
		shader.setUniform4f('Radius1', Radius1[0],Radius1[1],Radius1[2],Radius1[3])
		shader.setUniform4f('Radius2', Radius2[0],Radius2[1],Radius2[2],Radius2[3])
		shader.setUniform1f('windStrength', own['windStrength'])
		shader.setUniform1f('AmbientMulti', own['AmbientMulti'])
		shader.setUniform1f('LightMulti', own['LightMulti'])
		shader.setUniform1f('Intensity', own['Intensity']*2.0)
		shader.setUniform1f('LODdist', own['LODdist'])
		shader.setUniform1f('Time', own['Time'])
#endif
		ss->addUniform(new osg::Uniform("Radius1", osg::Vec4f(100.f, 100.f, 100.f, 100.f)));
		ss->addUniform(new osg::Uniform("Radius2", osg::Vec4f(100.f, 100.f, 1000.f, 1000.f)));
		ss->addUniform(new osg::Uniform("windStrength", 0.125f));
		ss->addUniform(new osg::Uniform("AmbientMulti", 0.025f));
		ss->addUniform(new osg::Uniform("LightMulti", 1.0f));
		ss->addUniform(new osg::Uniform("Intensity", 0.5f * 2.0));
		ss->addUniform(new osg::Uniform("LODdist", 24.0f));

#if 1

        //ss->setMode( GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
		//ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		//ss->setRenderBinDetails( 9/*RENDER_BIN_SCENE*/, "DepthSortedBin" );

		ss->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
#if 0
		ss->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
		ss->setAttribute( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.25f), osg::StateAttribute::ON );
#endif
		
		ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	    ss->setMode( GL_ALPHA_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

		// setup blending
		osg::BlendFunc * pBlendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
		ss->setAttributeAndModes(pBlendFunc, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

		osg::BlendEquation* pBlendEquation = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
		ss->setAttributeAndModes(pBlendEquation,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

		//ss->setNestRenderBins(false);
		//ss->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
#endif
	}

}

void createGRState(osg::Node * gr)
{
	osg::ref_ptr<osg::Program> program;
	program = createProgram("groundrender",vertexShaderSource,fragmentShaderSource);
	gr->getOrCreateStateSet()->setAttributeAndModes(program);	
	
	osg::StateSet* ss = gr->getOrCreateStateSet();

	osg::Texture2D* pTex1 = new osg::Texture2D;
	osg::Image* pImage1 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/GrassHeight1.png") );
	pTex1->setImage( pImage1 );
	ss->setTextureAttributeAndModes( 0, pTex1, osg::StateAttribute::ON );
	ss->addUniform( new osg::Uniform("Texture1"      , 0) );

	osg::Texture2D* pTex2 = new osg::Texture2D;
	osg::Image* pImage2 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/GroundGrass.Png") );
	pTex2->setImage( pImage2 );
	ss->setTextureAttributeAndModes( 1, pTex2, osg::StateAttribute::ON );
	ss->addUniform( new osg::Uniform("Texture2"      , 1) );
	pTex2->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
	pTex2->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );

	osg::Texture2D* pTex3 = new osg::Texture2D;
	osg::Image* pImage3 = osgDB::readImageFile( osgDB::findDataFile("blender/grass/GroundDirtyGrass.Png") );
	pTex3->setImage( pImage3 );
	ss->setTextureAttributeAndModes( 2, pTex3, osg::StateAttribute::ON );
	ss->addUniform( new osg::Uniform("Texture3"      , 2) );
	pTex3->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
	pTex3->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
}

int main_grass_test2( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);
	
	osg::setNotifyLevel( osg::INFO );

    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\models");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\areas");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\sky");
    osgDB::getDataFilePathList().push_back(osgDB::getCurrentWorkingDirectory() + "\\data\\materials\\lib");   

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
   
    osg::ref_ptr<osg::Node> sub_model = osgDB::readNodeFile( "blender/grass/ProceduralGrassShader_v3.dae" );

	FindNodeVisitor::nodeNamesList list_name;
	list_name.push_back("sky");
	list_name.push_back("obj");
	//list_name.push_back("groundgrass");
	//list_name.push_back("groundrender");

	FindNodeVisitor findNodes(list_name); 
	sub_model->accept(findNodes);

	const FindNodeVisitor::nodeListType& wln_list = findNodes.getNodeList();

	for(auto it = wln_list.begin(); it != wln_list.end(); ++it )
	{
		//(*it)->getOrCreateStateSet()->setAttribute(new osg::Program(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF);
	    (*it)->setNodeMask(0);
	}

	auto gg = findFirstNode(sub_model ,"GroundGrass",FindNodeVisitor::not_exact);
	_buildStateSet(gg);
	
	auto gr = findFirstNode(sub_model ,"GroundRender",FindNodeVisitor::not_exact);
	createGRState(gr);


    root->addChild(sub_model.get());

    viewer.apply(new osgViewer::SingleScreen(1));
    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}

AUTO_REG(main_grass_test2)