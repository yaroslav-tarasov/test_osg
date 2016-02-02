#include "stdafx.h"


namespace {

const char* vertCode = {

    "uniform int width;\n"
    "uniform int height;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "}\n"
};

const char* fragCode = {

    "uniform sampler2D defaultTex;\n"
    "void main()\n"
    "{\n"
	"    gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
    "}\n"
};

const char* vertMrtCode = {

	"uniform int width;\n"
	"uniform int height;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"}\n"
};

const char* fragMrtCode = {

	"uniform sampler2D defaultTex;\n"
	"void main()\n"
	"{\n"
	"    gl_FragData[0] = vec4(1.0,1.0,0.0,1.0); // texture2D(defaultTex, gl_TexCoord[0].xy);\n"
	"    gl_FragData[1] = vec4(0.0,0.0,1.0,1.0); //texture2D(defaultTex, gl_TexCoord[0].yx);\n"
	"}\n"
};


	osg::Node * createScene()
	{
		osg::Sphere* sphere    = new osg::Sphere( osg::Vec3( 0.f, 0.f, 0.f ), 0.25f );
		osg::ShapeDrawable* sd = new osg::ShapeDrawable( sphere );
		sd->setColor( osg::Vec4( 1.f, 0.f, 0.f, 1.f ) );
		sd->setName( "A nice sphere" );

		osg::Geode* geode = new osg::Geode;
		geode->addDrawable( sd );

		// Set material for basic lighting and enable depth tests. Else, the sphere
		// will suffer from rendering errors.
		{
			osg::StateSet* stateSet = geode->getOrCreateStateSet();
			osg::Material* material = new osg::Material;

			material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );	

			stateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
			stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );
		}
		

		osg::ref_ptr<osg::Image> image = osgDB::readImageFile("47.png");
		osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
		texture->setImage( image );
		texture->setInternalFormat( GL_RGBA32F_ARB );
		texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
		texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
		geode->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
		geode->getOrCreateStateSet()->addUniform( new osg::Uniform("defaultTex", 0) );
		geode->getOrCreateStateSet()->addUniform( new osg::Uniform("width", (int)image->s()) );
		geode->getOrCreateStateSet()->addUniform( new osg::Uniform("height", (int)image->t()) );




		return geode;
	}
}

#define NUM_TEXTURES 2
const size_t tex_width = 800;
const size_t tex_height = 600;

osg::Camera* createCamera()
// now create the camera to do the multiple render to texture
{
	// textures to render to and to use for texturing of the final quad
	osg::TextureRectangle* textureRect[NUM_TEXTURES];/// = {0,0,0,0};

	for (int i=0;i<NUM_TEXTURES;i++) {
		textureRect[i] = new osg::TextureRectangle;
		textureRect[i]->setTextureSize(tex_width, tex_height);
		textureRect[i]->setInternalFormat(GL_RGBA);
		textureRect[i]->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		textureRect[i]->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

#if 0
		if (useHDR)
		{
			// Default HDR format
			textureRect[i]->setInternalFormat(GL_RGBA32F_ARB);

			// GL_FLOAT_RGBA32_NV might be supported on pre 8-series GPUs
			//textureRect[i]->setInternalFormat(GL_FLOAT_RGBA32_NV);

			// GL_RGBA16F_ARB can be used with this example,
			// but modify e-12 and e12 in the shaders accordingly
			//textureRect[i]->setInternalFormat(GL_RGBA16F_ARB);

			textureRect[i]->setSourceFormat(GL_RGBA);
			textureRect[i]->setSourceType(GL_FLOAT);
		}
#endif
	}
	
	osg::Camera* camera = new osg::Camera;

	// set up the background color and clear mask.
	camera->setClearColor(osg::Vec4(0.1f,0.1f,0.3f,1.0f));
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// the camera is going to look at our input quad
	camera->setProjectionMatrix(osg::Matrix::ortho2D(-1,1,-1,1));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());

	// set viewport
	camera->setViewport(0, 0, tex_width, tex_height);

	// set the camera to render before the main camera.
	camera->setRenderOrder(osg::Camera::PRE_RENDER);

	// tell the camera to use OpenGL frame buffer objects
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	// attach the textures to use
	for (int i=0; i<NUM_TEXTURES; i++) {
			camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), textureRect[i]);


	}

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader( new osg::Shader(osg::Shader::VERTEX, vertMrtCode) );
	program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragMrtCode) );
	camera->getOrCreateStateSet()->setAttributeAndModes( program.get() );
	
	//// add the subgraph to render
	//camera->addChild(cam_subgraph);

	//parent->addChild(camera);

	return camera;
}

int main_mrt( int argc, char** argv )
{
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osg::Group> scene_parent = new osg::Group;
	osg::ref_ptr<osg::Node>  scene = createScene();
	scene_parent->addChild(  scene );

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader( new osg::Shader(osg::Shader::VERTEX, vertCode) );
	program->addShader( new osg::Shader(osg::Shader::FRAGMENT, fragCode) );
	scene_parent->getOrCreateStateSet()->setAttributeAndModes( program.get() );

    root->addChild(  scene_parent );
	osg::ref_ptr<osg::Camera> camera = createCamera();
	camera->addChild( scene  );
    root->addChild( camera );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}


AUTO_REG(main_mrt)