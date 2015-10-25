/**
* Adrian Egli (2010)
* OpenSceneGraph -> Screen Space Ambient Occlusion
*                   Cubic Spline Based Ambient Occlusion
*
*
*
*
*  -------------------------------------------------------------------
*    rendering pipeline (multi-pass rendering (4 pass)
*  -------------------------------------------------------------------
*
*   --> RTTInfo_NormalDepth (1)
*          |-> image processing          : RTTInfo_SSAO  (2)         --\
*          |                                                           |--> final rendering pass (lighting) (4)
*          \-> parallel split shadow map : RTTInfoShadow (3)         --/
*
*
*
*/
#include "stdafx.h"
#include "av/precompiled.h"

#include "ffssao.h"

#include <osgViewer/Viewer>

#include <osg/ClampColor>

#include "av/avShadows/ShadowedScene.h"
//#include <osgShadow/ParallelSplitShadowMap>
//#include <osgShadow/LightSpacePerspectiveShadowMap>
#include "av/avShadows/ParallelSplitShadowMap.h"

#include <osg/PolygonMode>

#include <osg/CullFace>

#include <sstream>

using namespace osgFX;


float SSAO::SUB_SAMPLING_FACTOR=1.0;

int SSAO::WIDTH  = 768;
int SSAO::HEIGHT = 768;

int SSAO::MAX_WIDTH  = 2048;
int SSAO::MAX_HEIGHT = 2048;



float SSAO::global_max_phi								= 90.000000000f;
float SSAO::global_max_radius							=  3.000000000f;
float SSAO::global_contrast								=  1.000000000f;
float SSAO::global_algPESSAO_contrastNB                 =  1.000000000f;
float SSAO::global_algPESSAO_LoopMax					= 30.000000000f;
float SSAO::global_algPESSAO_DEPTH_OFFSET				=  0.000000001f;
float SSAO::global_algPESSAO_COLOR_BLEND				=  0.000000000f;
float SSAO::global_algPESSAO_CamMoveAO					=  0.000000000f;
float SSAO::global_algPESSAO_RadiusScaleStepFactor		=  1.000000000f;
float SSAO::global_algPESSAO_InitRadiusInPixel		    =  0.050000000f;
float SSAO::global_algPESSAO_FastOff					=  0.000000000f;
float SSAO::global_algPESSAO_ArtifactsRemove_CNT        =  1.000000000f;
float SSAO::global_algPESSAO_SpeedUpILoop				=  0.150000000f;
float SSAO::global_algPESSAO_AttenuationFactor			=  0.800000000f;
float SSAO::global_algPESSAO_DepthCutOff			    =  0.030000000f;
float SSAO::global_algPESSAO_DepthCutOff_Const		    =  0.019000000f;

bool SSAO::global_debug_color_pssm = false;
int SSAO::global_shadow_show = 0;

//#define PRINT_OUT_INFO
static int ReceivesShadowTraversalMask = 0x1;
static int CastsShadowTraversalMask = 0x2;


//////////////////////////////////////////////////////////////////////////
class FragmentShaderGeneratorPSSM : public avShadow::ParallelSplitShadowMap::FragmentShaderGenerator {
public:
	// FragmentShaderGenerator
	virtual std::string generateGLSL_FragmentShader_BaseTex(
		bool debug,
		unsigned int splitCount,
		double textureRes,
		bool filtered,
		unsigned int nbrSplits,
		unsigned int textureOffset
		) {
			std::stringstream sstr;
			/// base texture
			sstr << "uniform sampler2D baseTexture; "      << std::endl;
			sstr << "uniform float enableBaseTexture; "     << std::endl;
			sstr << "uniform vec2 ambientBias;"    << std::endl;
			for (unsigned int i=0;i<nbrSplits;i++)    {
				sstr << "uniform sampler2DShadow shadowTexture"    <<    i    <<"; "    << std::endl;
				sstr << "uniform float zShadow"                    <<    i    <<"; "    << std::endl;
			}
			sstr << "uniform sampler2D samplerNormalDepth;\n"\
				"uniform vec3 imagePlaneLeftUniform;\n"\
				"uniform vec3 imagePlaneUpUniform;\n"\
				"  \n"\
				"vec4 readNormalDepth( in vec2 coord ) {\n"\
				"   vec4 d = texture2D(samplerNormalDepth,coord);\n"\
				"	vec3 normal = d.y*imagePlaneLeftUniform;\n"\
				"	normal     += d.z*imagePlaneUpUniform;\n"\
				"	return vec4(normalize(normal),d.x);\n"\
				"} \n"\
				"  \n"\
				"vec4 transformShadow(in vec4 shadow, in vec2 samplerRTSceneCoord) {\n"\
				"  vec4 normalDepth = readNormalDepth(samplerRTSceneCoord);\n"\
				"  return shadow;\n"\
				"}\n"\
				"  \n"\
				"float sum4( in vec4 v) {\n"\
				"  return v.x+v.y+v.z+v.w;\n"\
				"}\n"\
				"\n"\
				"float max4( in vec4 v) {\n"\
				"  float a=max(v.x,v.y);\n"\
				"  float b=max(v.z,v.w);\n"\
				"  return max(a,b);\n"\
				"}\n"\
				"  \n"\
				"float min4( in vec4 v) {\n"\
				"  float a=min(v.x,v.y);\n"\
				"  float b=min(v.z,v.w);\n"\
				"  return min(a,b);\n"\
				"}\n"\
				"  \n";

			sstr << "void main(void)"    << std::endl;
			sstr << "{"    << std::endl;
			/// select the shadow map : split
			sstr << "float testZ = gl_FragCoord.z*2.0-1.0;" <<std::endl;
			sstr << "float map0 = step(testZ, zShadow0);"<< std::endl;//DEBUG
			for (unsigned int i=1;i<nbrSplits;i++)    {
				sstr << "float map" << i << "  = step(zShadow"<<i-1<<",testZ)*step(testZ, zShadow"<<i<<");"<< std::endl;//DEBUG
			}
			if (filtered) {
				sstr << "          float fTexelSize="<< (1.0 / textureRes ) <<";" << std::endl;
				sstr << "          float fZOffSet  = -0.002954;" << std::endl; // good value for ATI / NVidia
			}
			for (unsigned int i=0;i<nbrSplits;i++)    {
				if (!filtered) {
					sstr << "    float shadow"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+textureOffset)    <<"]).r;"   << std::endl;
					sstr << " shadow"    <<    i    <<" = step(0.25,shadow"    <<    i    <<");" << std::endl; // reduce shadow artefacts
				} else {
					// filter the shadow (look up) 3x3
					//
					// 1 0 1
					// 0 2 0
					// 1 0 1
					//
					// / 6
					sstr << "    float shadowOrg"    <<    i    <<" = shadow2DProj( shadowTexture"  <<    i    <<",gl_TexCoord["    <<    (i+textureOffset)    <<"]+vec4(0.0,0.0,fZOffSet,0.0) ).r;"   << std::endl;
					sstr << "    float shadow0"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+textureOffset)    <<"]+vec4(-fTexelSize,-fTexelSize,fZOffSet,0.0) ).r;"   << std::endl;
					sstr << "    float shadow1"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+textureOffset)    <<"]+vec4( fTexelSize,-fTexelSize,fZOffSet,0.0) ).r;"   << std::endl;
					sstr << "    float shadow2"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+textureOffset)    <<"]+vec4( fTexelSize, fTexelSize,fZOffSet,0.0) ).r;"   << std::endl;
					sstr << "    float shadow3"    <<    i    <<" = shadow2DProj( shadowTexture"    <<    i    <<",gl_TexCoord["    <<    (i+textureOffset)    <<"]+vec4(-fTexelSize, fTexelSize,fZOffSet,0.0) ).r;"   << std::endl;
					sstr << "    float shadow"    <<    i    <<" = ( shadowOrg"    <<    i
						<<" + shadow0"    <<    i
						<<" + shadow1"    <<    i
						<<" + shadow2"    <<    i
						<<" + shadow3"    <<    i
						<< ")/4.0;"<< std::endl; //the div 4 is not a bug
					sstr << " vec4 s0123"<<i<<" = shadow"<<i<<"+shadowOrg"<<i<<"* vec4(shadow0"<< i << ",shadow1"<< i << ",shadow2"<< i << ",shadow3"<< i << ");" << std::endl;
					sstr << " shadow"<<i<<" = 0.5*sqrt((sum4(s0123"<<i<<") - max4(s0123"<<i<<") - min4(s0123"<<i<<"))/2.0);" << std::endl;
				}
			}
			sstr << "    float term0 = (1.0-shadow0)*map0; "    << std::endl;
			for (unsigned int i=1;i<nbrSplits;i++)    {
				sstr << "    float term" << i << " = map"<< i << "*(1.0-shadow"<<i<<");"<< std::endl;
			}
			/// build shadow factor value v
			sstr << "    float v = clamp(";
			for (unsigned int i=0;i<nbrSplits;i++)    {
				sstr << "term"    <<    i;
				if ( i+1 < nbrSplits ){
					sstr << "+";
				}
			}
			sstr << ",0.0,1.0);"    << std::endl;
			if ( debug ) {
				sstr << "    float c0=0.0;" << std::endl;
				sstr << "    float c1=0.0;" << std::endl;
				sstr << "    float c2=0.0;" << std::endl;
				sstr << "    float sumTerm=0.0;" << std::endl;
				for (unsigned int i=0;i<nbrSplits;i++)    {
					if ( i < 3 ) sstr << "    c" << i << "=term" << i << ";" << std::endl;
					sstr << "    sumTerm=sumTerm+term" << i << ";" << std::endl;
				}
				sstr << "    vec4 color    = ( 1.0 - sumTerm ) + (sumTerm)* vec4(c0,(1.0-c0)*c1,(1.0-c0)*(1.0-c1)*c2,1.0); "    << std::endl;
				switch(nbrSplits){
						case 1: sstr << "    color    =  color*0.75 + vec4(map0,0,0,1.0)*0.25; "    << std::endl;break;
						case 2: sstr << "    color    =  color*0.75 + vec4(map0,map1,0,1.0)*0.25; "    << std::endl;break;
						case 3: sstr << "    color    =  color*0.75 + vec4(map0,map1,map2,1.0)*0.25; "    << std::endl; break;
						case 4: sstr << "    color    =  color*0.75 + vec4(map0+map3,map1+map3,map2,1.0)*0.25; "    << std::endl; break;
						case 5: sstr << "    color    =  color*0.75 + vec4(map0+map3,map1+map3+map4,map2+map4,1.0)*0.25; "    << std::endl;break;
						case 6: sstr << "    color    =  color*0.75 + vec4(map0+map3+map5,map1+map3+map4,map2+map4+map5,1.0)*0.25; "    << std::endl;    break;
						default: break;
				}
			} else {
				//transformShadow
				sstr << "    vec4 shadow4 = vec4(0.5*v,0.5*v,0.5*v,1.0);"<< std::endl;
				sstr << "    vec4 color    = vec4(1.0,1.0,1.0,1.0) - shadow4; "<< std::endl;
			}


			sstr << "    vec4 colorTex = color;" << std::endl;
			sstr << "    gl_FragColor = color; "<< std::endl;
			sstr << "	 gl_FragColor = clamp(gl_FragColor,0.0,1.0);"<< std::endl;

			sstr << "}"<< std::endl;
			//std::cout << sstr.str() << std::endl;
			if ( splitCount == nbrSplits-1 )    osg::notify(osg::INFO) << std::endl << "ParallelSplitShadowMap: GLSL shader code:" << std::endl << "-------------------------------------------------------------------"  << std::endl << sstr.str() << std::endl;
			return sstr.str();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


osg::Switch* createShadowScene(osg::Node* scene, osg::LightSource* lightSource, bool debugColor=false) {
	osg::Switch* shadowSwitch = new osg::Switch;

	// empty no shadow
	osg::ref_ptr<osg::Geode> geodeEmpty = new osg::Geode;
	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::InfinitePlane());
	osg::Vec4 weigthColor(1.0,1.0,1.0,1.0);
	sd->setColor(weigthColor);
	//geodeEmpty->addDrawable(sd);
	geodeEmpty->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
	shadowSwitch->addChild(geodeEmpty,true);

	// parallel split shadow map
	for (int i=0;i<5;i++)
	{
		osg::ref_ptr<avShadow::ShadowedScene> shadowedScene = new avShadow::ShadowedScene;
		shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
		shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
		shadowedScene->addChild(scene);

		int mapres = 256;
		int mapcount = 1;
		switch (i){
case 0: mapres = 2048; mapcount = 1; break;
case 1: mapres = 2048; mapcount = 2; break;
case 2: mapres = 2048; mapcount = 3; break;
case 3: mapres = 2048; mapcount = 4; break;
	//default:
		};
		osg::ref_ptr<avShadow::ParallelSplitShadowMap> pssm = new avShadow::ParallelSplitShadowMap(NULL,mapcount);

		if ( debugColor ) pssm->setDebugColorOn();

		pssm->setTextureResolution(mapres);
		if ( lightSource ) pssm->setUserLight(lightSource->getLight());
		double polyoffsetfactor = pssm->getPolygonOffset().x();
		double polyoffsetunit   = pssm->getPolygonOffset().y();
		pssm->setPolygonOffset(osg::Vec2(polyoffsetfactor,polyoffsetunit));
		pssm->setMaxFarDistance(128.0);
		pssm->setFragmentShaderGenerator(new FragmentShaderGeneratorPSSM());

		pssm->enableShadowGLSLFiltering(true);

		osg::ref_ptr<osg::CullFace> cullFaceState= new osg::CullFace;
		shadowedScene->getOrCreateStateSet()->setAttributeAndModes(cullFaceState.get(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		shadowedScene->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		shadowedScene->setShadowTechnique(pssm.get());
		pssm->init();


		shadowSwitch->addChild(shadowedScene.get(),false);
	}

	return shadowSwitch;
}


osg::Node* createBase(osg::Texture2D* texture ,osg::Texture2D* texture2=NULL,osg::Texture2D* texture3=NULL,osg::Texture2D* texture4=NULL )
{

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	// set up the texture of the base.
	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
	if (texture2)stateset->setTextureAttributeAndModes(1,texture2,osg::StateAttribute::ON);
	if (texture3)stateset->setTextureAttributeAndModes(2,texture3,osg::StateAttribute::ON);
	if (texture4)stateset->setTextureAttributeAndModes(3,texture3,osg::StateAttribute::ON);
	geode->setStateSet( stateset.get() );


	osg::ref_ptr<osg::HeightField> grid = new osg::HeightField;
	grid->allocate(2,2);
	grid->setOrigin(osg::Vec3(0.0,0.0,0.0));
	grid->setXInterval(1.0);
	grid->setYInterval(1.0);

	grid->setHeight(0.0,0.0,-1.0);
	grid->setHeight(0.0,1.0,-1.0);
	grid->setHeight(1.0,1.0,-1.0);
	grid->setHeight(1.0,0.0,-1.0);

	geode->addDrawable(new osg::ShapeDrawable(grid.get()));

	osg::Group* group = new osg::Group;
	group->addChild(geode.get());


	// disable clamping
	//osg::ClampColor* clamp = new osg::ClampColor();
	//clamp->setClampVertexColor(GL_FALSE);
	//clamp->setClampFragmentColor(GL_FALSE);
	//clamp->setClampReadColor(GL_FALSE);
	//group->getOrCreateStateSet()->setAttribute(clamp, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);


	return group;

}

Registry::Proxy proxy(new SSAO);


DefaultTechnique::DefaultTechnique(
								   osg::Group* scene,
								   osg::Group* ssaoFX,
								   SSAO::SSAO_TYPE type,
								   osg::LightSource* ls,
								   std::string lightMap
								   ) :
Technique(),
_scene(scene),
_SSAO_type(type),
_LightSource(ls)
{

	_algPESSAO_LoopMaxUniform = new osg::Uniform("algPESSAO_LoopMaxUniform",SSAO::global_algPESSAO_LoopMax);
	_algPESSAO_DEPTH_OFFSETUniform = new osg::Uniform("algPESSAO_DEPTH_OFFSETUniform",SSAO::global_algPESSAO_DEPTH_OFFSET);
	_algPESSAO_COLOR_BLENDUniform = new osg::Uniform("algPESSAO_COLOR_BLENDUniform",SSAO::global_algPESSAO_COLOR_BLEND);
	_algPESSAO_CamMoveAOUniform = new osg::Uniform("algPESSAO_CamMoveAOUniform",SSAO::global_algPESSAO_CamMoveAO);
	_algPESSAO_InitRadiusInPixel = new osg::Uniform("algPESSAO_InitRadiusInPixel",SSAO::global_algPESSAO_InitRadiusInPixel);
	_algPESSAO_FastOff = new osg::Uniform("algPESSAO_FastOff",SSAO::global_algPESSAO_FastOff);
	_algPESSAO_ArtifactsRemove_CNT = new osg::Uniform("algPESSAO_ArtifactsRemove_CNT",SSAO::global_algPESSAO_ArtifactsRemove_CNT);
	_algPESSAO_SpeedUpILoop = new osg::Uniform("algPESSAO_SpeedUpILoop",SSAO::global_algPESSAO_SpeedUpILoop);
	_algPESSAO_AttenuationFactor = new osg::Uniform("algPESSAO_AttenuationFactor",SSAO::global_algPESSAO_AttenuationFactor);

	_algPESSAO_DepthCutOffUniform = new osg::Uniform("algPESSAO_DepthCutOffUniform",SSAO::global_algPESSAO_DepthCutOff);
	_algPESSAO_DepthCutOffUniform_Const = new osg::Uniform("algPESSAO_DepthCutOffUniform_Const",SSAO::global_algPESSAO_DepthCutOff_Const);
	_algPESSAO_RadiusScaleStepFactorUniform = new osg::Uniform("algPESSAO_RadiusScaleStepFactorUniform",SSAO::global_algPESSAO_RadiusScaleStepFactor);
	_algPESSAO_contrastUniformNB = new osg::Uniform("algPESSAO_contrastUniformNB", SSAO::global_algPESSAO_contrastNB);
	_contrastUniform = new osg::Uniform("contrastUniform",SSAO::global_contrast);
	_phiStepUniform = new osg::Uniform("phiStepUniform",SSAO::global_max_phi);
	_radiusMaxUniform = new osg::Uniform("radiusMaxUniform",SSAO::global_max_radius);

	_nearUniform = new osg::Uniform("nearUniform",FLT_MIN);
	_farUniform  = new osg::Uniform("farUniform",FLT_MAX);

	_imagePlaneLeft= new osg::Uniform("imagePlaneLeftUniform",osg::Vec3d(0.0,1.0,0.0));
	_imagePlaneUp  = new osg::Uniform("imagePlaneUpUniform",osg::Vec3d(0.0,0.0,1.0));


	////////////////////////////////////////////////////////////////////////// create all RTT cameras (Pass)
	_viewportWidth = 0;
	_viewportHeight = 0;
	createRTTInfoObjects( SSAO::WIDTH, SSAO::HEIGHT );

	////////////////////////////////////////////////////////////////////////// SSAO final pass

	osg::ref_ptr<osg::StateSet> ss = ssaoFX->getOrCreateStateSet();
	{
		for(unsigned int textLoop=0;textLoop<2;textLoop++){
			// fake texture for baseTexture, add a fake texture
			// we support by default at least one texture layer
			// without this fake texture we can not support
			// textured and not textured scene
			osg::ref_ptr<osg::Image> image = new osg::Image;
			// allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent to a Vec4!
			int noPixels = 1;
			image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
			image->setInternalTextureFormat(GL_RGBA);
			// fill in the image data.
			osg::Vec4* dataPtr = (osg::Vec4*)image->data();
			osg::Vec4f color(1.0f,1.0f,1.0f,1.0f);
			*dataPtr = color;
			// make fake texture
			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
			texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
			texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
			texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
			texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
			texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
			texture->setImage(image.get());
			// add fake texture
			ss->setTextureAttribute(textLoop,texture.get(),osg::StateAttribute::ON);
			ss->setTextureMode(textLoop,GL_TEXTURE_1D,osg::StateAttribute::OFF);
			ss->setTextureMode(textLoop,GL_TEXTURE_2D,osg::StateAttribute::ON);
			ss->setTextureMode(textLoop,GL_TEXTURE_3D,osg::StateAttribute::OFF);
		}
	}

	_RTTInfoSSAO._RTTTexture2Bind._textureID = 2;
	_RTTInfoSSAO._RTTTexture2Bind._stateSet2Bind = ss;
	ss->setTextureAttributeAndModes(_RTTInfoSSAO._RTTTexture2Bind._textureID ,_RTTInfoSSAO._texture.get(),osg::StateAttribute::ON);
	_RTTInfoShadow._RTTTexture2Bind._textureID = 3;
	_RTTInfoShadow._RTTTexture2Bind._stateSet2Bind = ss;
	ss->setTextureAttributeAndModes(_RTTInfoShadow._RTTTexture2Bind._textureID,_RTTInfoShadow._texture.get(),osg::StateAttribute::ON);

	_RTTInfoNormalDepth._RTTTexture2Bind._textureID = 4;
	_RTTInfoNormalDepth._RTTTexture2Bind._stateSet2Bind = ss;
	ss->setTextureAttributeAndModes(_RTTInfoNormalDepth._RTTTexture2Bind._textureID,_RTTInfoNormalDepth._texture.get(),osg::StateAttribute::ON);


	osg::ref_ptr<osg::Program> program = new osg::Program;
	ss->setAttribute(program.get());

	//////////////////////////////////////////////////////////////////////////
	// Uniform for texture
	osg::ref_ptr<osg::Uniform> samplerText0 = new osg::Uniform("samplerText0",0);
	ss->addUniform(samplerText0.get());
	osg::ref_ptr<osg::Uniform> samplerText1 = new osg::Uniform("samplerText1",1);
	ss->addUniform(samplerText1.get());
	osg::ref_ptr<osg::Uniform> samplerRTScene = new osg::Uniform("samplerRTScene",(int)_RTTInfoSSAO._RTTTexture2Bind._textureID);
	ss->addUniform(samplerRTScene.get());
	osg::ref_ptr<osg::Uniform> shadowSamplerUni = new osg::Uniform("samplerShadow",(int)_RTTInfoShadow._RTTTexture2Bind._textureID);
	ss->addUniform(shadowSamplerUni.get());
	osg::ref_ptr<osg::Uniform> depthNormalSamplerUni = new osg::Uniform("samplerNormalDepth",(int)_RTTInfoNormalDepth._RTTTexture2Bind._textureID);
	ss->addUniform(depthNormalSamplerUni.get());

	//////////////////////////////////////////////////////////////////////////
	// setup light map texture
	if ( ! lightMap.empty() ) {
		osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
		texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
		texture->setImage(osgDB::readImageFile(lightMap));
		texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);
		texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
		texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
		texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
		ss->setTextureAttributeAndModes(5,texture.get(),osg::StateAttribute::ON);
	} else {
		osg::ref_ptr<osg::Image> image = new osg::Image;
		// allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent to a Vec4!
		int noPixels = 1;
		image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA);
		// fill in the image data.
		osg::Vec4* dataPtr = (osg::Vec4*)image->data();
		osg::Vec4f color(1.0f,1.0f,1.0f,1.0f);
		*dataPtr = color;
		osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
		texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
		texture->setImage(image.get());
		texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);
		texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
		texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
		texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
		ss->setTextureAttributeAndModes(5,texture.get(),osg::StateAttribute::ON);
	}
	osg::ref_ptr<osg::Uniform> samplerLightHDR = new osg::Uniform("samplerLightHDR",5);
	ss->addUniform(samplerLightHDR.get());



	osg::Vec2f k(_RTTInfoNormalDepth._resolution_x,_RTTInfoNormalDepth._resolution_y );
	_textureSizeUniform = new osg::Uniform("textureSizeUniform",k);
	ss->addUniform(_textureSizeUniform.get());

	_textureSubSamplingFactor= new osg::Uniform("textureSubSamplingFactorUniform",SSAO::SUB_SAMPLING_FACTOR);
	ss->addUniform(_textureSubSamplingFactor.get());

	osg::Vec3f camDir(0.0,1.0,0.0);
	_camDirUniform = new osg::Uniform("camDirUniform",camDir);
	ss->addUniform(_camDirUniform.get());
	_lightDirUniform = new osg::Uniform("lightDirUniform",camDir);
	ss->addUniform(_lightDirUniform.get());

	ss->addUniform(_imagePlaneLeft.get());
	ss->addUniform(_imagePlaneUp.get());

	osg::Vec3f camPos(0.0,0.0,0.0);
	_camPosUniform = new osg::Uniform("camPosUniform",camPos);

	_camAspectRatioUniform = new osg::Uniform("camAspectRatioUniform",1.0f);
	osg::Vec2f mnfp(1.0f,1.0f);
	_metricNearFarPlanePixelResolutionUniform  = new osg::Uniform("metricNearFarPlanePixelResolutionUniform",mnfp);

	osg::Matrixd mat;mat.identity();
	_mvpwInvUniform = new osg::Uniform("mvpwInvUniform",mat);



	ss->addUniform(_algPESSAO_COLOR_BLENDUniform.get());
	ss->addUniform(_algPESSAO_LoopMaxUniform.get());


	ss->addUniform(_algPESSAO_ArtifactsRemove_CNT.get());


	ss->addUniform(_algPESSAO_CamMoveAOUniform.get());


	if ( _SSAO_type == SSAO::DEFAULT_SSAO  ) {
		osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
			"uniform sampler2D samplerRTScene;\n"\
			"uniform sampler2D samplerText0;\n"\
			"uniform sampler2D samplerText1;\n"\
			"uniform sampler2D samplerShadow;\n"\
			"uniform float contrastUniform;\n"\
			"uniform float textureSubSamplingFactorUniform;\n"\
			"uniform vec2 textureSizeUniform;\n"\
			"uniform float algPESSAO_DEPTH_OFFSETUniform;\n"\
			"uniform float algPESSAO_COLOR_BLENDUniform;\n"\
			"\n"\
			"float sum4( in vec4 v) {\n"\
			"  return v.x+v.y+v.z+v.w;\n"\
			"}\n"\
			"\n"\
			"float max4( in vec4 v) {\n"\
			"  float a=max(v.x,v.y);\n"\
			"  float b=max(v.z,v.w);\n"\
			"  return max(a,b);\n"\
			"}\n"\
			"\n"\
			"float min4( in vec4 v) {\n"\
			"  float a=min(v.x,v.y);\n"\
			"  float b=min(v.z,v.w);\n"\
			"  return min(a,b);\n"\
			"}\n"\
			"\n"\
			"void main(void)\n"\
			"{\n"\
			"   vec4 text0 = texture2D(samplerText0,gl_TexCoord[0].st);\n"\
			"   vec4 text1 = texture2D(samplerText1,gl_TexCoord[1].st);\n"\
			"	vec2 samplerRTSceneCoord =  gl_FragCoord.xy/textureSizeUniform/textureSubSamplingFactorUniform;\n"\
			"	vec4 shadow = texture2D(samplerShadow,   samplerRTSceneCoord);\n"\
			"	vec4 poiSSAO = texture2D(samplerRTScene, samplerRTSceneCoord);\n"\
			"   float ssao = poiSSAO.x;\n"\
			"	vec4 ssao4;\n"\
			"	float off = 5.0*pow(1.0-poiSSAO.z,3.0);\n"\
			"   ssao4.x  = texture2D(samplerRTScene, vec2((gl_FragCoord.x-off)/textureSizeUniform.x, (gl_FragCoord.y+off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.y  = texture2D(samplerRTScene, vec2((gl_FragCoord.x-off)/textureSizeUniform.x, (gl_FragCoord.y-off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.z  = texture2D(samplerRTScene, vec2((gl_FragCoord.x+off)/textureSizeUniform.x, (gl_FragCoord.y-off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.w  = texture2D(samplerRTScene, vec2((gl_FragCoord.x+off)/textureSizeUniform.x, (gl_FragCoord.y+off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao = sum4(ssao4)+ssao-2.0*max(ssao,max4(ssao4)) - 2.0*min(ssao,min4(ssao4));\n"\
			"	vec4 colorText =clamp(text0*text1*gl_Color,0.0,1.0);\n"\
			"	vec4 color = (1.0-algPESSAO_COLOR_BLENDUniform)+algPESSAO_COLOR_BLENDUniform*colorText;\n"\
			"	gl_FragColor.rgb = color.rgb*ssao;\n"\
			"	if ( algPESSAO_COLOR_BLENDUniform > 1.01 ) {\n"\
			"		float ao2 = 1.1-(algPESSAO_COLOR_BLENDUniform-1.0);\n"\
			"		color = (1.0-ao2)+ao2*colorText;\n"\
			"		if ( samplerRTSceneCoord.x > 0.33 ) gl_FragColor.rgb = color.rgb*vec3(ssao,ssao,ssao); \n"\
			"		if ( samplerRTSceneCoord.x > 0.66 ) gl_FragColor.rgb = vec3(ssao,ssao,ssao); \n"\
			"		if ( samplerRTSceneCoord.x < 0.33 ) gl_FragColor.rgb = color.rgb; \n"\
			"		if ( algPESSAO_COLOR_BLENDUniform < 1.2 ) gl_FragColor.rgb = color.rgb; \n"\
			"	}\n"\
			"	gl_FragColor.rgb *= shadow.rgb;\n"\
			"	gl_FragColor.a = colorText.a;\n"\
			"}"
			);
		program->addShader(fragment_shader.get());
	} else if ( ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO || _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED )  )  {

		//////////////////////////////////////////////////////////////////////////
		// vertex shader
		osg::ref_ptr<osg::Shader> vertex_shader = new osg::Shader(osg::Shader::VERTEX,
			"varying vec3 vNormal;\n"\
			"varying vec3 vNormalStat;\n"\
			"varying vec3 vCenterDir;\n"\
			"varying vec3 lightVec_L0;\n"\
			"varying vec3 halfVector_L0;\n"\
			"varying vec3 lightVec_L1;\n"\
			"varying vec3 halfVector_L1;\n"\
			"\n"\
			"uniform vec3 camPosUniform;\n"\
			"uniform vec3 camDirUniform;\n"\
			" \n"\
			"void main(void)\n"\
			"{\n"\
			"   vNormalStat = normalize(gl_Normal);\n"\
			"   vNormal = normalize(gl_NormalMatrix * gl_Normal);\n"\
			"   \n"\
			"   gl_FrontColor   = gl_Color;\n"\
			"	gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"\
			"   \n"\
			"   gl_TexCoord[0] = gl_MultiTexCoord0;\n"\
			"   gl_TexCoord[1] = gl_MultiTexCoord1;\n"\
			"   \n"\
			"   mat3 trans = mat3(vec3(gl_NormalMatrix[0].x,gl_NormalMatrix[1].x,gl_NormalMatrix[2].x),vec3(gl_NormalMatrix[0].y,gl_NormalMatrix[1].y,gl_NormalMatrix[2].y),vec3(gl_NormalMatrix[0].z,gl_NormalMatrix[1].z,gl_NormalMatrix[2].z));\n"\
			"   vec3 tangent = normalize(gl_NormalMatrix * trans[0]); \n"\
			"   vec3 binormal = normalize(gl_NormalMatrix * trans[1]);\n"\
			"	\n"\
			"	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);\n"\
			"   vec3 tmpVec = gl_LightSource[0].position.xyz - vVertex;\n"\
			"   lightVec_L0.x = dot(tmpVec, tangent);\n"\
			"   lightVec_L0.y = dot(tmpVec, binormal);\n"\
			"   lightVec_L0.z = dot(tmpVec, vNormal);\n"\
			"   halfVector_L0 = reflect(-tmpVec, gl_Normal);\n"\
			"   halfVector_L0.x = dot(tmpVec, tangent);\n"\
			"   halfVector_L0.y = dot(tmpVec, binormal);\n"\
			"   halfVector_L0.z = dot(tmpVec, vNormal);\n"\
			"   tmpVec = gl_LightSource[1].position.xyz - vVertex;\n"\
			"   lightVec_L1.x = dot(tmpVec, tangent);\n"\
			"   lightVec_L1.y = dot(tmpVec, binormal);\n"\
			"   lightVec_L1.z = dot(tmpVec, vNormal);\n"\
			"   halfVector_L1 = reflect(-tmpVec, gl_Normal);\n"\
			"   halfVector_L1.x = dot(tmpVec, tangent);\n"\
			"   halfVector_L1.y = dot(tmpVec, binormal);\n"\
			"   halfVector_L1.z = dot(tmpVec, vNormal);\n"\
			"	\n"\
			"}"
			);
		program->addShader(vertex_shader.get());

		//////////////////////////////////////////////////////////////////////////
		// Fragment shader
		osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
			"uniform sampler2D samplerRTScene;\n"\
			"uniform sampler2D samplerText0;\n"\
			"uniform sampler2D samplerText1;\n"\
			"uniform sampler2D samplerShadow;\n"\
			"uniform sampler2D samplerLightHDR;\n"\
			"uniform vec3 lightDirUniform;\n"\
			"uniform vec3 camPosUniform;\n"\
			"uniform vec3 camDirUniform;\n"\
			"uniform vec2 textureSizeUniform;\n"\
			"uniform float textureSubSamplingFactorUniform;\n"\
			"uniform float algPESSAO_DEPTH_OFFSETUniform;\n"\
			"uniform float algPESSAO_COLOR_BLENDUniform;\n"\
			"uniform float farUniform;\n"\
			"uniform float algPESSAO_ArtifactsRemove_CNT;\n"\
			"\n"\
			"varying vec3 vNormal;\n"\
			"varying vec3 vNormalStat;\n"\
			"varying vec3 vCenterDir;\n"\
			"varying vec3 lightVec_L0;\n"\
			"varying vec3 halfVector_L0;\n"\
			"varying vec3 lightVec_L1;\n"\
			"varying vec3 halfVector_L1;\n"\
			"\n"\
			"const float strength = 1.0;\n"\
			"\n"\
			"float sum4( in vec4 v) {\n"\
			"  return v.x+v.y+v.z+v.w;\n"\
			"}\n"\
			"\n"\
			"float max4( in vec4 v) {\n"\
			"  float a=max(v.x,v.y);\n"\
			"  float b=max(v.z,v.w);\n"\
			"  return max(a,b);\n"\
			"}\n"\
			"  \n"\
			"float min4( in vec4 v) {\n"\
			"  float a=min(v.x,v.y);\n"\
			"  float b=min(v.z,v.w);\n"\
			"  return min(a,b);\n"\
			"}\n"\
			"  \n"\
			"  \n"\
			"  \n"\
			"void main(void)\n"\
			"{\n"\
			"	vec2 samplerRTSceneFactor =  1.0/textureSizeUniform/textureSubSamplingFactorUniform;\n"\
			"	vec2 samplerRTSceneCoord =  gl_FragCoord.xy*samplerRTSceneFactor;\n"\
			"	\n"\
			"	vec4 shadow  = texture2D(samplerShadow,   samplerRTSceneCoord);\n"\
			"	\n"\
			"	vec2 texCoord0 = gl_TexCoord[0].st;\n"\
			"	vec2 texCoord1 = gl_TexCoord[1].st;\n"\
			"	\n"\
			"	vec4 poiSSAO = texture2D(samplerRTScene, samplerRTSceneCoord);\n"\
			"   float ssao = poiSSAO.x;\n"\
			"	vec4 ssao4;\n"\
			"	float off = max(0.0,1.0-(algPESSAO_ArtifactsRemove_CNT-1.0)/2.0) * (0.25+0.75*pow(1.0-poiSSAO.z,3.0));\n"\
			"   ssao4.x  = texture2D(samplerRTScene, vec2((gl_FragCoord.x-off)/textureSizeUniform.x, (gl_FragCoord.y+off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.y  = texture2D(samplerRTScene, vec2((gl_FragCoord.x-off)/textureSizeUniform.x, (gl_FragCoord.y-off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.z  = texture2D(samplerRTScene, vec2((gl_FragCoord.x+off)/textureSizeUniform.x, (gl_FragCoord.y-off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.w  = texture2D(samplerRTScene, vec2((gl_FragCoord.x+off)/textureSizeUniform.x, (gl_FragCoord.y+off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao = sum4(ssao4)+2.0*ssao-max(ssao,max4(ssao4)) - min(ssao,min4(ssao4));\n"\
			"	ssao /= 4.0;\n"\
			"	\n"\
			"	vec4 text0 = texture2D(samplerText0,texCoord0);\n"\
			"   vec4 text1 = texture2D(samplerText1,texCoord1);\n"\
			"	\n"\
			"	vec3 vVDir = normalize( -lightDirUniform );\n"\
			"	vec3 vNDir = normalize( vNormalStat );\n"\
			"	float rho =	0.5*(1.0+dot(vNDir,vVDir));\n"\
			"	float phi = ssao;\n"\
			"	vec2 vLight = shadow.xy*vec2(rho,phi);\n"\
			"	vec4 light = clamp(ssao*texture2D(samplerLightHDR,vLight),0.0,1.0);\n"\
			"	\n"\
			"	\n"\
			"	vec3 n = normalize(vNormal);\n"\
			"	vec3 l0 = normalize(lightVec_L0);\n"\
			"	vec3 h0 = normalize(halfVector_L0);\n"\
			"	float nDotL = max(0.0, dot(n, l0));\n"\
			"	float nDotH = max(0.0, dot(n, h0));\n"\
			"	float power = (nDotL == 0.0) ? 0.0 : pow(nDotH, gl_FrontMaterial.shininess);\n"\
			"	\n"\
			"	vec4 ambient = gl_FrontLightProduct[0].ambient * ssao * shadow.r;\n"\
			"	vec4 diffuse = gl_FrontLightProduct[0].diffuse * gl_FrontMaterial.diffuse * nDotL * shadow.r;\n"\
			"	vec4 specular = gl_FrontLightProduct[0].specular * gl_FrontMaterial.specular * power * strength;\n"\
			"	vec4 color = clamp(gl_FrontLightModelProduct.sceneColor  + ambient + diffuse + specular,0.0,1.0);\n"\
			"	\n"\
			"	vec3 l1 = normalize(lightVec_L1);\n"\
			"	vec3 h1 = normalize(halfVector_L1);\n"\
			"	nDotL = max(0.0, dot(n, l1));\n"\
			"	nDotH = max(0.0, dot(n, h1));\n"\
			"	power = (nDotL == 0.0) ? 0.0 : pow(nDotH, gl_FrontMaterial.shininess);\n"\
			"	ambient  = gl_FrontLightProduct[1].ambient * ssao * shadow.r;\n"\
			"	diffuse  = gl_FrontLightProduct[1].diffuse * gl_FrontMaterial.diffuse * nDotL * shadow.r;\n"\
			"	specular = gl_FrontLightProduct[1].specular * gl_FrontMaterial.specular * power * strength;\n"\
			"	color += clamp( ambient + diffuse + specular,0.0,1.0);\n"\
			"	\n"\
			"	color += gl_FrontLightModelProduct.sceneColor;\n"\
			"	\n"\
			"	vec4 colorText = clamp(1.25*(color * text0 * text1 * ssao),0.0,1.0);\n"\
			"	\n"\
 			"	color = (1.0-algPESSAO_COLOR_BLENDUniform)+algPESSAO_COLOR_BLENDUniform*colorText;\n"\
			"	gl_FragColor.rgb = color.rgb*ssao;\n"\
			"	if ( algPESSAO_COLOR_BLENDUniform > 1.01 ) {\n"\
			"		float ao2 = 1.1-(algPESSAO_COLOR_BLENDUniform-1.0);\n"\
			"		color = (1.0-ao2)+ao2*colorText;\n"\
			"		if ( samplerRTSceneCoord.x > 0.33 ) gl_FragColor.rgb = color.rgb; \n"\
			"		if ( samplerRTSceneCoord.x > 0.66 ) gl_FragColor.rgb = vec3(ssao,ssao,ssao); \n"\
			"		if ( samplerRTSceneCoord.x < 0.33 ) gl_FragColor.rgb = color.rgb/vec3(ssao,ssao,ssao); \n"\
			"		if ( algPESSAO_COLOR_BLENDUniform < 1.2 ) gl_FragColor.rgb = color.rgb; \n"\
			"	}\n"\
			"	gl_FragColor = shadow*gl_FragColor;\n"\
 			"	gl_FragColor.a = colorText.a;\n"\
			"}"
			);
		program->addShader(fragment_shader.get());

	} else {

		osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
			"uniform sampler2D samplerRTScene;\n"\
			"uniform sampler2D samplerText0;\n"\
			"uniform sampler2D samplerText1;\n"\
			"uniform sampler2D samplerShadow;\n"\
			"uniform float contrastUniform;\n"\
			"uniform float textureSubSamplingFactorUniform;\n"\
			"uniform vec2 textureSizeUniform;\n"\
			"uniform float algPESSAO_DEPTH_OFFSETUniform;\n"\
			"uniform float algPESSAO_COLOR_BLENDUniform;\n"\
			"\n"\
			"\n"\
			"float sum4( in vec4 v) {\n"\
			"  return v.x+v.y+v.z+v.w;\n"\
			"}\n"\
			"\n"\
			"float max4( in vec4 v) {\n"\
			"  float a=max(v.x,v.y);\n"\
			"  float b=max(v.z,v.w);\n"\
			"  return max(a,b);\n"\
			"}\n"\
			"\n"\
			"float min4( in vec4 v) {\n"\
			"  float a=min(v.x,v.y);\n"\
			"  float b=min(v.z,v.w);\n"\
			"  return min(a,b);\n"\
			"}\n"\
			"\n"\
			"void main(void)\n"\
			"{\n"\
			"	vec2 samplerRTSceneCoord =  gl_FragCoord.xy/textureSizeUniform/textureSubSamplingFactorUniform;\n"\
			"	vec4 shadow = texture2D(samplerShadow,   samplerRTSceneCoord);\n"\
			"   vec4 text0 = texture2D(samplerText0,gl_TexCoord[0].st);\n"\
			"   vec4 text1 = texture2D(samplerText1,gl_TexCoord[1].st);\n"\
			"	vec4 poiSSAO = texture2D(samplerRTScene, samplerRTSceneCoord);\n"\
			"   float ssao1 = poiSSAO.x;\n"\
			"	vec4 ssao4;\n"\
			"	float off = 2.0*pow(1.0-poiSSAO.z,3.0);\n"\
			"   ssao4.x  = texture2D(samplerRTScene, vec2((gl_FragCoord.x-off)/textureSizeUniform.x, (gl_FragCoord.y+off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.y  = texture2D(samplerRTScene, vec2((gl_FragCoord.x-off)/textureSizeUniform.x, (gl_FragCoord.y-off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.z  = texture2D(samplerRTScene, vec2((gl_FragCoord.x+off)/textureSizeUniform.x, (gl_FragCoord.y-off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao4.w  = texture2D(samplerRTScene, vec2((gl_FragCoord.x+off)/textureSizeUniform.x, (gl_FragCoord.y+off)/textureSizeUniform.y)/textureSubSamplingFactorUniform).x;\n"\
			"	ssao1 = sum4(ssao4)+ssao1-max(ssao1,max4(ssao4)) - min(ssao1,min4(ssao4));\n"\
			"	ssao1 /= 3.0;\n"\
			"	vec4 ssao = vec4(ssao1,ssao1,ssao1,1.0);\n"\
			"	vec4 colorText =clamp(text0*text1*gl_Color,0.0,1.0);\n"\
			"	vec4 color = (1.0-algPESSAO_COLOR_BLENDUniform)+algPESSAO_COLOR_BLENDUniform*colorText;\n"\
			"	gl_FragColor.rgb = color.rgb*ssao.rgb;\n"\
			"	if ( algPESSAO_COLOR_BLENDUniform > 1.01 ) {\n"\
			"		float ao2 = 1.1-(algPESSAO_COLOR_BLENDUniform-1.0);\n"\
			"		color = (1.0-ao2)+ao2*colorText;\n"\
			"		if ( samplerRTSceneCoord.x > 0.33  ) gl_FragColor.rgb = color.rgb*ssao.rgb; \n"\
			"		if ( samplerRTSceneCoord.x > 0.66  ) gl_FragColor.rgb = ssao.rgb; \n"\
			"		if ( samplerRTSceneCoord.x < 0.33  ) gl_FragColor.rgb = color.rgb; \n"\
			"		if ( algPESSAO_COLOR_BLENDUniform < 1.2 ) gl_FragColor.rgb = color.rgb; \n"\
			"   }\n"\
			"	gl_FragColor.rgb *= shadow.rgb;\n"\
			"	gl_FragColor.a = colorText.a;\n"\
			"}"
			);
		program->addShader(fragment_shader.get());
	}

}

void DefaultTechnique::getRequiredExtensions(std::vector<std::string>& extensions)
{
}

bool DefaultTechnique::validate(osg::State& state) const
{
	if (!Technique::validate(state)) return false;

	//osg::TextureCubeMap::Extensions *ext = osg::TextureCubeMap::getExtensions(state.getContextID(), true);
	//if (ext) {
	//	return ext->isCubeMapSupported();
	//}
	return true;
}


/** optional: return a node that overrides the child node on a specified pass */
osg::Node* DefaultTechnique::getOverrideChild(int passNum)  {

	if ( passNum == 0 ) { // Depth
		return _RTTInfoNormalDepth._camera.get();
	}
	if ( passNum == 1 ) { // SSAO
		return _RTTInfoSSAO._camera.get();
	}
	if ( passNum == 2 ) { // Shadow pass
		return _RTTInfoShadow._camera.get();
	}

	return 0;

}

void DefaultTechnique::createRTTInfoObjects(unsigned int viewportWidth, unsigned int viewportHeight) {
	_viewportWidth  = viewportWidth;
	_viewportHeight = viewportHeight;

	_RTTInfoNormalDepth._resolution_x = viewportWidth  / SSAO::SUB_SAMPLING_FACTOR;
	_RTTInfoNormalDepth._resolution_y = viewportHeight / SSAO::SUB_SAMPLING_FACTOR;

	// texture
	_RTTInfoNormalDepth._texture = new osg::Texture2D;
	_RTTInfoNormalDepth._texture->setTextureSize(_RTTInfoNormalDepth._resolution_x,_RTTInfoNormalDepth._resolution_y);
	_RTTInfoNormalDepth._texture->setInternalFormat(GL_RGBA32F_ARB);
	_RTTInfoNormalDepth._texture->setSourceFormat(GL_RGBA);
	_RTTInfoNormalDepth._texture->setSourceType(GL_FLOAT);
	_RTTInfoNormalDepth._texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
	_RTTInfoNormalDepth._texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
	_RTTInfoNormalDepth._texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
	_RTTInfoNormalDepth._texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
	_RTTInfoNormalDepth._texture->setResizeNonPowerOfTwoHint(false);


	// camera
	_RTTInfoNormalDepth._camera = new osg::Camera;
	_RTTInfoNormalDepth._camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
	_RTTInfoNormalDepth._camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_RTTInfoNormalDepth._camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	// set viewport
	_RTTInfoNormalDepth._camera->setViewport(0,0,_RTTInfoNormalDepth._resolution_x,_RTTInfoNormalDepth._resolution_y);
	// set the camera to render before the main camera.
	_RTTInfoNormalDepth._camera->setRenderOrder(osg::Camera::PRE_RENDER);
	// tell the camera to use OpenGL frame buffer object where supported.
	_RTTInfoNormalDepth._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	// attach texture
	_RTTInfoNormalDepth._camera->attach(osg::Camera::COLOR_BUFFER0, _RTTInfoNormalDepth._texture.get(),0,0);

	// add subgraph to render
	_RTTInfoNormalDepth._camera->addChild(_scene.get());

	// force polygon mode (fill)
	osg::ref_ptr<osg::PolygonMode> polyModeObj_Pass1 = new osg::PolygonMode;
	polyModeObj_Pass1->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL);
	_RTTInfoNormalDepth._camera->getOrCreateStateSet()->setAttribute(polyModeObj_Pass1.get());

	////////////////////////////////////////////////////////////////////////// SSAO pass
	// texture resolution
	_RTTInfoSSAO._resolution_x =  _RTTInfoNormalDepth._resolution_x;
	_RTTInfoSSAO._resolution_y =  _RTTInfoNormalDepth._resolution_y;

	// texture
	_RTTInfoSSAO._texture = new osg::Texture2D;
	_RTTInfoSSAO._texture->setTextureSize(_RTTInfoSSAO._resolution_x,_RTTInfoSSAO._resolution_y);
	_RTTInfoSSAO._texture->setInternalFormat(GL_RGBA32F_ARB);
	_RTTInfoSSAO._texture->setSourceFormat(GL_RGBA);
	_RTTInfoSSAO._texture->setSourceType(GL_FLOAT);
	_RTTInfoSSAO._texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
	_RTTInfoSSAO._texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
	_RTTInfoSSAO._texture->setResizeNonPowerOfTwoHint(false);


	// camera
	_RTTInfoSSAO._camera = new osg::Camera;
	_RTTInfoSSAO._camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
	_RTTInfoSSAO._camera->setClearMask( GL_DEPTH_BUFFER_BIT);
	_RTTInfoSSAO._camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	// set viewport
	_RTTInfoSSAO._camera->setViewport(0,0,_RTTInfoSSAO._resolution_x,_RTTInfoSSAO._resolution_y);
	_RTTInfoSSAO._camera->setViewMatrixAsLookAt(osg::Vec3(0.5,0.5,1),osg::Vec3(0.5,0.5,0),osg::Vec3(0,1,0));
	_RTTInfoSSAO._camera->setProjectionMatrixAsOrtho(-0.5,0.5,-0.5,0.5,0.0,100000.0);

	// set the camera to render before the main camera.
	_RTTInfoSSAO._camera->setRenderOrder(osg::Camera::PRE_RENDER);
	// tell the camera to use OpenGL frame buffer object where supported.
	_RTTInfoSSAO._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	// attach texture (SSAO filter)
	_RTTInfoSSAO._camera->attach(osg::Camera::COLOR_BUFFER0, _RTTInfoSSAO._texture.get(),0,0);

	// force polygon mode (fill)
	osg::ref_ptr<osg::PolygonMode> polyModeObj_SSAO = new osg::PolygonMode;
	polyModeObj_SSAO->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL);
	_RTTInfoSSAO._camera->getOrCreateStateSet()->setAttribute(polyModeObj_SSAO.get());

	// add subgraph to render
	_RTTInfoSSAO._baseTexture2DNode = createBase( _RTTInfoNormalDepth._texture.get() );
	_RTTInfoSSAO._camera->addChild(_RTTInfoSSAO._baseTexture2DNode.get());

	////////////////////////////////////////////////////////////////////////// Shadow pass
	// texture resolution
	_RTTInfoShadow._resolution_x =  _RTTInfoSSAO._resolution_x;
	_RTTInfoShadow._resolution_y =  _RTTInfoSSAO._resolution_y;

	// texture
	_RTTInfoShadow._texture = new osg::Texture2D;
	_RTTInfoShadow._texture->setTextureSize(_RTTInfoShadow._resolution_x,_RTTInfoShadow._resolution_y);
	_RTTInfoShadow._texture->setInternalFormat(GL_RGBA32F_ARB);
	_RTTInfoShadow._texture->setSourceFormat(GL_RGBA);
	_RTTInfoShadow._texture->setSourceType(GL_FLOAT);
	_RTTInfoShadow._texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
	_RTTInfoShadow._texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
	_RTTInfoShadow._texture->setResizeNonPowerOfTwoHint(false);

	// camera
	_RTTInfoShadow._camera = new osg::Camera;
	_RTTInfoShadow._camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	_RTTInfoShadow._camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	_RTTInfoShadow._camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

	// set viewport
	_RTTInfoShadow._camera->setViewport(0,0,_RTTInfoShadow._resolution_x,_RTTInfoShadow._resolution_y);
	_RTTInfoShadow._camera->setViewMatrixAsLookAt(osg::Vec3(0.5,0.5,1),osg::Vec3(0.5,0.5,0),osg::Vec3(0,1,0));
	_RTTInfoSSAO._camera->setProjectionMatrixAsOrtho(-0.5,0.5,-0.5,0.5,0.0,100000.0);

	// set the camera to render before the main camera.
	_RTTInfoShadow._camera->setRenderOrder(osg::Camera::PRE_RENDER);
	// tell the camera to use OpenGL frame buffer object where supported.
	_RTTInfoShadow._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	// attach texture (SSAO filter)
	_RTTInfoShadow._camera->attach(osg::Camera::COLOR_BUFFER0, _RTTInfoShadow._texture.get(),0,0);

	// force polygon mode (fill)
	osg::ref_ptr<osg::PolygonMode> polyModeObj_Shadow = new osg::PolygonMode;
	polyModeObj_Shadow->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL);
	_RTTInfoShadow._camera->getOrCreateStateSet()->setAttribute(polyModeObj_Shadow.get());

	// add subgraph to render
	_debug_color_pssm = SSAO::global_debug_color_pssm;
	_show_shadow_node = createShadowScene(_scene.get(),_LightSource.get(),_debug_color_pssm);
	osg::ref_ptr<osg::Group> shadowGrp = new osg::Group;
	shadowGrp->addChild(_show_shadow_node.get());
	_RTTInfoShadow._camera->addChild(shadowGrp.get());

	//////////////////////////////////////////////////////////////////////////
	{
		osg::ref_ptr<osg::StateSet> ss = _show_shadow_node->getOrCreateStateSet();

		ss->setTextureAttributeAndModes(8 ,_RTTInfoNormalDepth._texture.get(),osg::StateAttribute::ON);
		osg::ref_ptr<osg::Uniform> depthNormalSamplerUni = new osg::Uniform("samplerNormalDepth",8);
		ss->addUniform(depthNormalSamplerUni.get());
		ss->addUniform(_imagePlaneLeft.get());
		ss->addUniform(_imagePlaneUp.get());
		ss->addUniform(_camDirUniform.get());
		ss->addUniform(_camPosUniform.get());
	}



}

void DefaultTechnique::traverse(osg::NodeVisitor& nv, Effect* fx) {

	if ( this->_show_shadow_node.valid() ) {
		_show_shadow_node->setAllChildrenOff();
		_show_shadow_node->setSingleChildOn((unsigned int)SSAO::global_shadow_show);
	}
	if ( _algPESSAO_DEPTH_OFFSETUniform.valid() ) _algPESSAO_DEPTH_OFFSETUniform->set(SSAO::global_algPESSAO_DEPTH_OFFSET);
	if ( _algPESSAO_COLOR_BLENDUniform.valid() ) _algPESSAO_COLOR_BLENDUniform->set(SSAO::global_algPESSAO_COLOR_BLEND);
	if ( _algPESSAO_CamMoveAOUniform.valid() ) _algPESSAO_CamMoveAOUniform->set(SSAO::global_algPESSAO_CamMoveAO);
	if ( _algPESSAO_InitRadiusInPixel.valid() ) _algPESSAO_InitRadiusInPixel->set(SSAO::global_algPESSAO_InitRadiusInPixel);
	if ( _algPESSAO_FastOff.valid() ) _algPESSAO_FastOff->set(SSAO::global_algPESSAO_FastOff);
	if ( _algPESSAO_ArtifactsRemove_CNT.valid() ) _algPESSAO_ArtifactsRemove_CNT->set(SSAO::global_algPESSAO_ArtifactsRemove_CNT);
	if ( _algPESSAO_SpeedUpILoop.valid() ) _algPESSAO_SpeedUpILoop->set(SSAO::global_algPESSAO_SpeedUpILoop);
	if ( _algPESSAO_AttenuationFactor.valid() ) _algPESSAO_AttenuationFactor->set(SSAO::global_algPESSAO_AttenuationFactor);
	if ( _algPESSAO_DepthCutOffUniform.valid() ) _algPESSAO_DepthCutOffUniform->set(SSAO::global_algPESSAO_DepthCutOff);
	if ( _algPESSAO_DepthCutOffUniform_Const.valid() ) _algPESSAO_DepthCutOffUniform_Const->set(SSAO::global_algPESSAO_DepthCutOff_Const);
	if ( _algPESSAO_RadiusScaleStepFactorUniform.valid() ) _algPESSAO_RadiusScaleStepFactorUniform->set(SSAO::global_algPESSAO_RadiusScaleStepFactor);
	if ( _algPESSAO_LoopMaxUniform.valid() ) _algPESSAO_LoopMaxUniform->set(SSAO::global_algPESSAO_LoopMax);
	if ( _contrastUniform.valid() ) _contrastUniform->set((float)pow(0.125*(double)SSAO::global_contrast,0.25));
	if ( _algPESSAO_contrastUniformNB.valid() ) _algPESSAO_contrastUniformNB->set(0.2f*SSAO::global_algPESSAO_contrastNB);
	if ( _phiStepUniform.valid() )  _phiStepUniform->set(SSAO::global_max_phi);
	if ( _radiusMaxUniform.valid()) _radiusMaxUniform->set(SSAO::global_max_radius);


	// special actions must be taken if the node visitor is actually a CullVisitor
	osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
	if ( cv ) {
		osgUtil::RenderStage* orig_rs = cv->getRenderStage();

		osg::ref_ptr<osg::Viewport> vp = cv->getRenderInfo().getView()->getCamera()->getViewport();
		osg::ref_ptr<osg::Viewport> vpCam = _RTTInfoShadow._camera->getViewport();

		if ( SSAO::global_debug_color_pssm != _debug_color_pssm || _viewportWidth!= vp->width() || _viewportHeight != vp->height() || _subSamplingFactor != SSAO::SUB_SAMPLING_FACTOR ) {

			std::cout << "render window size or subsampling factor has changed." <<std::endl;
			std::cout << ">>" << 	vp->width()  << ":" <<  vp->height()  << ":" <<  SSAO::SUB_SAMPLING_FACTOR << std::endl;

			//////////////////////////////////////////////////////////////////////////
			// Resize the render-to-texture according to the viewports size by
			// recreating the RTT objects (texture and cameras)
			//////////////////////////////////////////////////////////////////////////
			_viewportWidth  = vp->width();
			_viewportHeight = vp->height();
			_subSamplingFactor = SSAO::SUB_SAMPLING_FACTOR;

			_textureSubSamplingFactor->set(SSAO::SUB_SAMPLING_FACTOR);

			createRTTInfoObjects(_viewportWidth,_viewportHeight);

			_RTTInfoSSAO._RTTTexture2Bind._stateSet2Bind->setTextureAttributeAndModes(_RTTInfoSSAO._RTTTexture2Bind._textureID ,_RTTInfoSSAO._texture.get(),osg::StateAttribute::ON);
			_RTTInfoShadow._RTTTexture2Bind._stateSet2Bind->setTextureAttributeAndModes(_RTTInfoShadow._RTTTexture2Bind._textureID ,_RTTInfoShadow._texture.get(),osg::StateAttribute::ON);
			_RTTInfoNormalDepth._RTTTexture2Bind._stateSet2Bind->setTextureAttributeAndModes(_RTTInfoNormalDepth._RTTTexture2Bind._textureID ,_RTTInfoNormalDepth._texture.get(),osg::StateAttribute::ON);




			//////////////////////////////////////////////////////////////////////////
			float x = (_RTTInfoNormalDepth._resolution_x);
			float y = (_RTTInfoNormalDepth._resolution_y);
			osg::Vec2f k(x,y);
			_textureSizeUniform->set(k);

		}


		_RTTInfoNormalDepth._camera->setViewMatrix(cv->getRenderInfo().getView()->getCamera()->getViewMatrix());
		_RTTInfoNormalDepth._camera->setProjectionMatrix(cv->getRenderInfo().getView()->getCamera()->getProjectionMatrix());



		_RTTInfoShadow._camera->setViewMatrix(cv->getRenderInfo().getView()->getCamera()->getViewMatrix());
		_RTTInfoShadow._camera->setProjectionMatrix(cv->getRenderInfo().getView()->getCamera()->getProjectionMatrix());


		double fovy,aspectRatio,zNear,zFar;
		cv->getRenderInfo().getView()->getCamera()->getProjectionMatrix().getPerspective(fovy,aspectRatio,zNear,zFar);
		zFar  *= 2.0;
		zNear /= 2.0;
		osg::Vec3 eye,center,up;
		cv->getRenderInfo().getView()->getCamera()->getViewMatrixAsLookAt(eye,center,up);

		osg::Vec3 camDir = center - eye;
		camDir.normalize();


		SSAO::global_algPESSAO_CamMoveAO = 1.0;
		if ((_cameraPos-eye).length()==0.0 && (_cameraDir-camDir).length() == 0.0 ) {
			SSAO::global_algPESSAO_CamMoveAO = 0.0;
		}
		_cameraPos = eye;
		_cameraDir = camDir;

		osg::Vec3 testYvec3(0.0,1.0,0.0);

		osg::Vec3 ipLeft = camDir^up;ipLeft.normalize();
		_imagePlaneLeft->set(ipLeft);
		_imagePlaneUp->set(up);

		if ( _nearUniform.valid() ) {
			_nearUniform->set((float)zNear);
#ifdef PRINT_OUT_INFO
			std::cout << "_nearUniform : " << zNear << std::endl;
#endif
		}

		if ( _farUniform.valid() )  {
			float curFar = (float)(zFar);
			_farUniform->set(curFar);
#ifdef PRINT_OUT_INFO
			std::cout << "_farUniform : " << zFar << std::endl;
#endif
		}


		if ( _lightDirUniform.valid() ) {
			_lightDirUniform->set(_LightSource->getLight()->getDirection());
		}

		if ( _camDirUniform.valid() ) {
			_camDirUniform->set(camDir);
		}

		if ( _camPosUniform.valid() ) {
			_camPosUniform->set(eye);
		}

		osg::Camera* camera = cv->getRenderInfo().getView()->getCamera();

		if ( _camAspectRatioUniform.valid() ) _camAspectRatioUniform->set((float)aspectRatio);
		if ( _metricNearFarPlanePixelResolutionUniform.valid() ) {
			float angleRad = osg::DegreesToRadians(fovy);
			osg::Vec2f metricNearFarPlanePixelResolution = osg::Vec2f(
				tan(angleRad)*zNear / (float)(camera->getViewport()->height()),
				tan(angleRad)*zFar  / (float)(camera->getViewport()->height())
				);
			_metricNearFarPlanePixelResolutionUniform->set(metricNearFarPlanePixelResolution);
		}

		osg::Matrixd  modelViewMtx = camera->getViewMatrix(),
			projectionMtx = camera->getProjectionMatrix(),
			viewPortMtx = camera->getViewport()->computeWindowMatrix();

		osg::Vec3d modelPosition = modelViewMtx.getTrans();

		osg::Matrixd mtx(modelViewMtx * projectionMtx * viewPortMtx);
		osg::Matrixd iMtx;iMtx.invert(mtx);
#ifdef PRINT_OUT_INFO
		std::cout << modelPosition[0] <<  " " << modelPosition[1] << " " << modelPosition[2] << std::endl;
		osg::Vec4 v1 = osg::Vec4(-1.0,-1.0,-1.0,1.0) * mtx;
		osg::Vec4 v2 = osg::Vec4( 1.0, 1.0, 1.0,1.0) * mtx;
		std::cout << ":1:\t"<< v1[0]/v1[3] <<  " " << v1[1]/v1[3] << " " << v1[2]/v1[3] << std::endl;
		std::cout << ":2:\t"<< v2[0]/v2[3] <<  " " << v2[1]/v2[3] << " " << v2[2]/v2[3] << std::endl;
		osg::Vec4 va = v2  - v1;
		{
			osg::Vec3 v(v1[0]/v1[3],v1[1]/v1[3],v1[2]/v1[3]);
			std::cout << v.length() << std::endl;
			v = v * iMtx;
			std::cout << v[0] <<  " " << v[1] << " " << v[2] << std::endl;
		}
		{
			osg::Vec3 v(v2[0]/v2[3],v2[1]/v2[3],v2[2]/v2[3]);
			std::cout << v.length() << std::endl;
			v = v * iMtx;
			std::cout << v[0] <<  " " << v[1] << " " << v[2] << std::endl;
		}
		{
			osg::Vec3 v(va[0],va[1],va[2]);
			std::cout << v.length() << std::endl;
			v = v * iMtx;
			std::cout << v[0] <<  " " << v[1] << " " << v[2]  << std::endl;
		}
#endif
		if ( _mvpwInvUniform.valid()) {
			_mvpwInvUniform->set(mtx);
		}
	}


	traverse_implementation(nv, fx);

}


void DefaultTechnique::define_passes()
{



	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// PASS 1
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	{   //  Normal / Depth Pass 1
		osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
		{
			unsigned int textLoop(0);
			// fake texture for baseTexture, add a fake texture
			// we support by default at least one texture layer
			// without this fake texture we can not support
			// textured and not textured scene

			osg::ref_ptr<osg::Image> image = new osg::Image;
			// allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent to a Vec4!
			int noPixels = 1;
			image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
			image->setInternalTextureFormat(GL_RGBA);
			// fill in the image data.
			osg::Vec4* dataPtr = (osg::Vec4*)image->data();
			osg::Vec4f color(1.0f,1.0f,1.0f,0.0f);
			*dataPtr = color;
			// make fake texture
			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
			texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
			texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
			texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
			texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
			texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
			texture->setImage(image.get());
			// add fake texture
			ss->setTextureAttribute(textLoop,texture.get(),osg::StateAttribute::ON);
			ss->setTextureMode(textLoop,GL_TEXTURE_1D,osg::StateAttribute::OFF);
			ss->setTextureMode(textLoop,GL_TEXTURE_2D,osg::StateAttribute::ON);
			ss->setTextureMode(textLoop,GL_TEXTURE_3D,osg::StateAttribute::OFF);
		}
		osg::ref_ptr<osg::Uniform> samplerText = new osg::Uniform("samplerText",0);
		ss->addUniform(samplerText.get());


		ss->addUniform(_nearUniform.get());
		ss->addUniform(_farUniform.get());
		ss->addUniform(_camDirUniform.get());

		ss->addUniform(_imagePlaneLeft);
		ss->addUniform(_imagePlaneUp);

		osg::ref_ptr<osg::Program> program = new osg::Program;
		ss->setAttribute(program.get());

		if (  _SSAO_type == SSAO::DEFAULT_SSAO ) {
			osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
				"uniform sampler2D samplerText;\n"\
				"uniform float farUniform;\n"\
				" \n"\
				"void main() {\n"\
				"   vec4 text = texture2D(samplerText,gl_TexCoord[0].st);\n"\
				"	float d = 1.0-0.95*(log(gl_FragCoord.z+0.25)-log(1.25))/(log(0.25)-log(1.25));\n"\
				"	gl_FragColor = vec4(d,d,d,step(0.9,gl_FrontMaterial.diffuse.a));\n"\
				"} "
				);
			program->addShader(fragment_shader.get());
		} else if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO  || _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED) {

			osg::ref_ptr<osg::Shader> vertex_shader = new osg::Shader(osg::Shader::VERTEX,
				"varying vec3 vNormal;\n"\
				"uniform float farUniform;\n"\
				"uniform float nearUniform;\n"\
				"uniform vec3 camPosUniform;\n"\
				"uniform vec3 camDirUniform;\n"\
				"varying float depthValue;\n"\
				" \n"\
				"void main(void)\n"\
				"{\n"\
				"	vec4 v = (gl_ModelViewMatrix * gl_Vertex);\n"\
				"   vNormal = normalize(gl_NormalMatrix * gl_Normal);\n"\
				"	gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"\
				"   depthValue = farUniform-(camPosUniform.z-v.z);		\n"\
				"}"
				);
			program->addShader(vertex_shader.get());



			std::stringstream fragShaderSS;
			fragShaderSS <<
				"varying float depthValue;\n"\
				"uniform float farUniform;\n"\
				"uniform vec3 imagePlaneLeftUniform;\n"\
				"uniform vec3 imagePlaneUpUniform;\n"\
				" \n"\
				"varying vec3 vNormal;\n"\
				" \n"\
				"void main() {\n";
			if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED )
				fragShaderSS <<
				"	float l = dot(imagePlaneLeftUniform,vNormal);"
				"	float u = dot(imagePlaneUpUniform,vNormal);"
				"	gl_FragData[0] = vec4( depthValue ,l,u,step(0.9,gl_FrontMaterial.diffuse.a));\n"
				" \n";
			if ( _SSAO_type != SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED )
				fragShaderSS <<
				"	gl_FragData[0] = vec4( depthValue ,0.0,0.0,step(0.9,gl_FrontMaterial.diffuse.a));\n"
				" \n";
			fragShaderSS <<
				"} ";
			osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragShaderSS.str());
			program->addShader(fragment_shader.get());
		} else {
			osg::ref_ptr<osg::Shader> vertex_shader = new osg::Shader(osg::Shader::VERTEX,
				"varying vec3 vNormal;\n"\
				" \n"\
				"void main(void)\n"\
				"{\n"\
				"   vNormal = normalize(gl_NormalMatrix * gl_Normal);\n"\
				"	gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"\
				"   gl_TexCoord[0] = gl_MultiTexCoord0;\n"\
				"}"
				);
			program->addShader(vertex_shader.get());
			osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
				"uniform sampler2D samplerText;\n"\
				"varying vec3 vNormal;\n"\
				" \n"\
				"void main() {\n"\
				"   vec4 text = texture2D(samplerText,gl_TexCoord[0].st);\n"\
				"	vec3 n = (gl_FragCoord.z*(normalize(vNormal))+1.0)/2.0;\n"\
				"	gl_FragColor = vec4(n,step(0.9,gl_FrontMaterial.diffuse.a));\n"\
				"} "
				);
			program->addShader(fragment_shader.get());
		}


		addPass(ss.get());
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// PASS 2
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	std::cout << _SSAO_type << std::endl;
	if ( _SSAO_type == SSAO::FFNBSSAO ) {
		osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;


		osg::ref_ptr<osg::Program> program = new osg::Program;
		ss->setAttribute(program.get());

		osg::ref_ptr<osg::Uniform> samplerNormalDepth = new osg::Uniform("samplerNormalDepth",0);
		ss->addUniform(samplerNormalDepth.get());


		ss->addUniform(_nearUniform.get());
		ss->addUniform(_farUniform.get());

		ss->addUniform(_algPESSAO_RadiusScaleStepFactorUniform.get());

		ss->addUniform(_contrastUniform.get());
		ss->addUniform(_phiStepUniform.get());
		ss->addUniform(_radiusMaxUniform.get());

		ss->addUniform(_textureSizeUniform.get());


		osg::ref_ptr<osg::Shader> vertex_shader = new osg::Shader(osg::Shader::VERTEX,
			"varying vec2 vTexCoord;\n"\
			"uniform float farUniform;\n"\
			"uniform float nearUniform;\n"\
			"void main(void)\n"\
			"{\n"\
			"	gl_Position = ftransform();\n"\
			"	vec2 Pos = gl_Vertex.xy;\n"\
			"	vTexCoord = Pos;\n"\
			"}"
			);
		program->addShader(vertex_shader.get());



		osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
			"uniform float osg_FrameTime;\n"\
			"uniform sampler2D samplerNormalDepth;\n"\
			"uniform float farUniform;\n"\
			"uniform float nearUniform;\n"\
			"uniform float phiStepUniform;\n"\
			"uniform float radiusMaxUniform;\n"\
			"uniform float contrastUniform;\n"\
			"uniform vec2 textureSizeUniform;\n"\
			"uniform float algPESSAO_RadiusScaleStepFactorUniform;\n"\
			"  \n"\
			"varying vec2 vTexCoord;\n"\
			"  \n"\
			"vec4 readNormalDepth( in vec2 coord ) {\n"\
			"	vec3 normal = texture2D(samplerNormalDepth,coord).xyz;\n"\
			"	normal = (2.0*normal)-1.0;\n"\
			"	return vec4(normalize(normal),length(normal));\n"\
			"}\n"\
			"  \n"\
			"void main(void)\n"\
			"{\n"\
			"	\n"\
			"	vec4 nd0 = readNormalDepth(vTexCoord.xy);\n"\
			"	vec4 nd2;\n"\
			"	\n"\
			"	float rnd = sin( 3.1415*mod( 123456.0*((2.0*gl_FragCoord.y/textureSizeUniform.y-1.0) *(2.0*gl_FragCoord.x/textureSizeUniform.x-1.0)*nd0.w+gl_FragCoord.x/textureSizeUniform.x * gl_FragCoord.y/textureSizeUniform.y),16.0)/16.0 );\n"\
			"	\n"\
			"	float nd0_W_inv      = pow(1.0-nd0.w,1.05);\n"\
			"	float LOD_sel_phi    = 1.0/max((4.0*nd0_W_inv),1.0);\n"\
			"	float fRadiusMax     = (algPESSAO_RadiusScaleStepFactorUniform*radiusMaxUniform);\n"\
			"	float LOD_sel_radius = fRadiusMax / max(ceil((fRadiusMax/4.0)*pow(nd0.w,1.15)),1.0);\n"\
			"   \n"\
			"	float radiusStep  = 1.41;\n"\
			"	float sampleCount = 0.0;\n"\
			"	float ao          = 0.0;\n"\
			"   \n"\
			"	float radiusMaxUniformLOD = min(max(LOD_sel_radius*(1.0+0.5*abs(rnd)),4.0),fRadiusMax) + radiusStep ;\n"\
			"	float phiStepUniformLod   = (1.0/algPESSAO_RadiusScaleStepFactorUniform*phiStepUniform) * LOD_sel_phi*(1.0+0.5*rnd);\n"\
			"   \n"\
			"	float fPhi= (1.0-step(nd0.w,1.0))*360.0;\n"\
			"   while ( fPhi < 360.0 ) "
			"   {\n"\
			"	  float sinfPhi =  sin (0.0175 * fPhi)/textureSizeUniform.x;\n"\
			"	  float cosfPhi =  cos (0.0175 * fPhi)/textureSizeUniform.y;\n"\
			"     float maxAO=1.0;\n"\
			"     float sumAO=0.0;\n"\
			"     float kCount=0.0;\n"\
			"     \n"\
			"	  float fRadius=radiusStep;\n"\
			"     \n"\
			"     vec2 offsetXY = fRadius*vec2(sinfPhi,cosfPhi);\n"\
			"     nd2 = readNormalDepth(vTexCoord.xy+offsetXY);\n"\
			"	  \n"\
			"	  float rejectCount=0.0;\n"\
			"	  \n"\
			"     while (fRadius < radiusMaxUniformLOD  )\n"\
			"     {\n"\
			"       offsetXY = fRadius*vec2(sinfPhi,cosfPhi);\n"\
			"       nd2 = readNormalDepth(vTexCoord.xy+offsetXY);\n"\
			"		\n"\
			"		float angleDiff = dot(nd0.xyz,nd2.xyz);\n"\
			"		maxAO = min( angleDiff, maxAO);\n"\
			"		\n"\
			"		kCount += 1.0;\n"\
			"       sumAO  += maxAO;\n"\
			"		\n"\
			"	    rejectCount += (1.0-step(nd2.w,nd0.w));\n"\
			"		fRadius += radiusStep * (pow(radiusStep,2.0*rejectCount));\n"\
			"		\n"\
			"     }\n"\
			"	  \n"\
			"	  float sDepth = step(nd2.w,nd0.w);\n"\
			"     sampleCount += 1.0;\n"\
			"	  \n"\
			"	  ao += ( (sumAO/kCount) * sDepth + (1.0-sDepth) );\n"\
			"	  \n"\
			"	  fPhi+=phiStepUniformLod;\n"\
			"   }\n"\
			"   ao = 0.5+ 0.5*pow(clamp(ao/sampleCount,0.0,1.0),pow(contrastUniform,0.25));\n"\
			"	gl_FragColor = vec4(ao,ao,ao,1.0);\n"\
			"}"
			);

		program->addShader(fragment_shader.get());
		addPass(ss.get());
	} // SSAO::FFNBSSAO
	else {
		if ( _SSAO_type == SSAO::DEFAULT_SSAO ) {   // DEFAULT SSAO 2
			osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

			osg::ref_ptr<osg::Program> program = new osg::Program;
			ss->setAttribute(program.get());

			osg::ref_ptr<osg::Uniform> samplerNormalDepth = new osg::Uniform("samplerNormalDepth",0);
			ss->addUniform(samplerNormalDepth.get());


			ss->addUniform(_nearUniform.get());
			ss->addUniform(_farUniform.get());


			ss->addUniform(_contrastUniform.get());
			ss->addUniform(_phiStepUniform.get());
			ss->addUniform(_radiusMaxUniform.get());
			ss->addUniform(_textureSizeUniform.get());


			osg::ref_ptr<osg::Shader> vertex_shader = new osg::Shader(osg::Shader::VERTEX,
				"varying vec2 texCoord;\n"\
				"uniform float farUniform;\n"\
				"uniform float nearUniform;\n"\
				"void main(void)\n"\
				"{\n"\
				"	gl_Position = ftransform();\n"\
				"	vec2 Pos = gl_Vertex.xy;\n"\
				"	texCoord = Pos;\n"\
				"}"
				);
			program->addShader(vertex_shader.get());



			osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,
				"uniform sampler2D samplerNormalDepth;\n"\
				"\n"\
				"uniform float farUniform;\n"\
				"uniform float nearUniform;\n"\
				"uniform float contrastUniform;\n"\
				"uniform vec2 textureSizeUniform;\n"\
				"\n"\
				"varying vec2 texCoord;\n"\
				"\n"\
				"vec4 readNormalDepth( in vec2 coord ) {\n"\
				"	vec3 normal = texture2D(samplerNormalDepth,coord).xyz;\n"\
				"	return vec4(normalize(normal),normal.x);\n"\
				"}\n"\
				"\n"\
				"float readDepth( in vec2 coord ) {\n"\
				"   vec2 screensize = textureSizeUniform;\n"\
				"	vec2 camerarange = vec2(nearUniform,farUniform);\n"\
				"	\n"\
				"	return  (2.0 * camerarange.x) / (camerarange.y + camerarange.x - readNormalDepth(coord).w * (camerarange.y - camerarange.x));\n"\
				"}\n"\
				"\n"\
				"float compareDepths( in float depth1, in float depth2 , in float aoMultiplier) {\n"\
				"   vec2 screensize = textureSizeUniform;\n"\
				"	vec2 camerarange = vec2(nearUniform,farUniform);\n"\
				"	\n"\
				"	float aoCap = 1.0;\n"\
				"	float depthTolerance=0.000125;\n"\
				"	float aorange = 10.0;/* units in space the AO effect extends to (this gets divided by the camera far range */ \n"\
				"	float diff = sqrt( clamp(1.0-(depth1-depth2) / (aorange/(camerarange.y-camerarange.x)),0.0,1.0) );\n"\
				"	float ao = min(aoCap,max(0.0,depth1-depth2-depthTolerance) * aoMultiplier) * diff;\n"\
				"	return ao;\n"\
				"}\n"\
				"\n"\
				"void main(void)\n"\
				"{\n"\
				"   vec2 screensize = textureSizeUniform;\n"\
				"	vec2 camerarange = vec2(nearUniform,farUniform);\n"\
				"	\n"\
				"	float depth = readDepth( texCoord );\n"\
				"	float d;\n"\
				"	\n"\
				"	float pw = 1.0 / screensize.x;\n"\
				"	float ph = 1.0 / screensize.y;\n"\
				"\n"\
				"	float aoMultiplier=1000.0;\n"\
				"\n"\
				"	float ao = 0.0;\n"\
				"	\n"\
				"	float aoscale=1.0;\n"\
				"\n"\
				"   float q = (0.78*sin(textureSizeUniform.x*(depth+texCoord.y+texCoord.x+(texCoord.x*texCoord.y))));\n"\
				"	vec2 v1 = vec2(cos(q),sin(q));\n"\
				"\n"\
				"	d=readDepth( v1*vec2(pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"	\n"\
				"	pw*=2.0;\n"\
				"	ph*=2.0;\n"\
				"	aoMultiplier/=2.0;\n"\
				"	aoscale*=1.2;\n"\
				"	\n"\
				"	\n"\
				"   q = (0.39*sin(textureSizeUniform.y*(ao+depth+texCoord.y+texCoord.x+(texCoord.x*texCoord.y))));\n"\
				"	v1 = vec2(cos(q),sin(q));\n"\
				"	\n"\
				"	d=readDepth( v1*vec2(pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	pw*=2.0;\n"\
				"	ph*=2.0;\n"\
				"	aoMultiplier/=2.0;\n"\
				"	aoscale*=1.2;\n"\
				"	\n"\
				"	\n"\
				"   q = (0.78*cos(textureSizeUniform.x*(ao+depth+texCoord.y+texCoord.x+(texCoord.x*texCoord.y))));\n"\
				"	v1 = vec2(cos(q),sin(q));\n"\
				"	\n"\
				"	d=readDepth( v1*vec2(pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"	\n"\
				"	pw*=2.0;\n"\
				"	ph*=2.0;\n"\
				"	aoMultiplier/=2.0;\n"\
				"	aoscale*=1.2;\n"\
				"	\n"\
				"	\n"\
				"   q = (0.39*cos(textureSizeUniform.x*(ao+depth+texCoord.y+texCoord.x+(texCoord.x*texCoord.y))));\n"\
				"	v1 = vec2(cos(q),sin(q));\n"\
				"	\n"\
				"	d=readDepth( v1*vec2(pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	d=readDepth( v1*vec2(-pw,-ph)+vec2(texCoord.x,texCoord.y));\n"\
				"	ao+=compareDepths(depth,d,aoMultiplier)/aoscale;\n"\
				"\n"\
				"	ao/=16.0;\n"\
				"	\n"\
				"	gl_FragColor = vec4(vec3(pow(1.0-ao,pow(contrastUniform,0.25))),1.0);\n"\
				"}	"
				);
			program->addShader(fragment_shader.get());
			addPass(ss.get());

		} // POLY SSAO  2
		else if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO  || _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED ) {
			osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;


			osg::ref_ptr<osg::Program> program = new osg::Program;
			ss->setAttribute(program.get());

			osg::ref_ptr<osg::Uniform> samplerDepth = new osg::Uniform("samplerNormalDepth",0);
			ss->addUniform(samplerDepth.get());

			ss->addUniform(_algPESSAO_DEPTH_OFFSETUniform.get());
			ss->addUniform(_algPESSAO_CamMoveAOUniform.get());
			ss->addUniform(_algPESSAO_InitRadiusInPixel.get());
			ss->addUniform(_algPESSAO_DepthCutOffUniform.get());
			ss->addUniform(_algPESSAO_DepthCutOffUniform_Const.get());
			ss->addUniform(_algPESSAO_RadiusScaleStepFactorUniform.get());
			ss->addUniform(_algPESSAO_contrastUniformNB.get());

			ss->addUniform(_contrastUniform.get());
			ss->addUniform(_farUniform.get());
			ss->addUniform(_nearUniform.get());
			ss->addUniform(_algPESSAO_LoopMaxUniform.get());
			ss->addUniform(_camDirUniform.get());

			ss->addUniform(_imagePlaneLeft.get());
			ss->addUniform(_imagePlaneUp.get());
			ss->addUniform(_camAspectRatioUniform.get());
			ss->addUniform(_metricNearFarPlanePixelResolutionUniform.get());

			ss->addUniform(_algPESSAO_FastOff.get());
			ss->addUniform(_algPESSAO_ArtifactsRemove_CNT.get());
			ss->addUniform(_algPESSAO_SpeedUpILoop.get());
			ss->addUniform(_algPESSAO_AttenuationFactor.get());

			ss->addUniform(_phiStepUniform.get());
			ss->addUniform(_radiusMaxUniform.get());

			osg::ref_ptr<osg::Shader> vertex_shader = new osg::Shader(osg::Shader::VERTEX,
				"varying vec2 vTexCoord;\n"\
				"void main(void)\n"\
				"{\n"\
				"	gl_Position = ftransform();\n"\
				"	vec2 Pos = gl_Vertex.xy;\n"\
				"	vTexCoord = Pos;\n"\
				"}"
				);
			program->addShader(vertex_shader.get());


			std::stringstream fragShaderSS;

			fragShaderSS <<
				"uniform float phiStepUniform;\n"\
				"uniform float radiusMaxUniform;\n"\
				"uniform sampler2D samplerNormalDepth;\n"\
				"uniform int osg_FrameNumber;\n"\
				"uniform float contrastUniform;\n"\
				"uniform float algPESSAO_LoopMaxUniform;\n"\
				"uniform vec2 textureSizeUniform;\n"\
				"uniform float textureSubSamplingFactorUniform;\n"\
				"uniform float farUniform;\n"\
				"uniform float nearUniform;\n"\
				"uniform float algPESSAO_contrastUniformNB;\n"\
				"uniform float algPESSAO_DEPTH_OFFSETUniform;\n"\
				"uniform float algPESSAO_CamMoveAOUniform;\n"\
				"uniform float algPESSAO_InitRadiusInPixel;\n"\
				"uniform float algPESSAO_DepthCutOffUniform_Const;\n"\
				"uniform float algPESSAO_DepthCutOffUniform;\n"\
				"uniform float algPESSAO_RadiusScaleStepFactorUniform;\n"\
				"uniform float algPESSAO_FastOff;\n"\
				"uniform float algPESSAO_ArtifactsRemove_CNT;\n"\
				"uniform float algPESSAO_SpeedUpILoop;\n"\
				"uniform float algPESSAO_AttenuationFactor;\n"\
				"uniform vec3 camDirUniform;\n"\
				"uniform vec3 imagePlaneLeftUniform;\n"\
				"uniform vec3 imagePlaneUpUniform;\n"\
				"uniform float camAspectRatioUniform;\n"\
				"uniform vec2 metricNearFarPlanePixelResolutionUniform;\n"\
				"  \n"\
				"varying vec2 vTexCoord;\n"\
				"  \n"\
				"  // spline with continuity C2\n"\
				"  const mat4 a1_c2 = mat4( 0.091652, -0.259036, 0.065835, 0.601549, 0.072391, -0.119529, -0.162458, -0.106061, -0.097676, 0.213855, 0.084768, -0.200947, -0.042421, 0.054687, 0.034006, -0.033646);  \n"\
				"  const mat4 a2_c2 = mat4( 0.601549, 0.065835, -0.259036, 0.091652, 0.106061, 0.162458, 0.119529, -0.072391, -0.200947, 0.084768, 0.213855, -0.097676, -0.048798, 0.012121, 0.042903, -0.018852);    \n"\
				"  const mat4 b1_c2 = mat4( 0.091652, -0.259036, 0.065835, 0.601549, 0.072391, -0.119529, -0.162458, -0.106061, -0.097676, 0.213855, 0.084768, -0.200947, 0.018852, -0.042903, -0.012121, 0.048798);  \n"\
				"  const mat4 b2_c2 = mat4( 0.601549, 0.065835, -0.259036, 0.091652, 0.106061, 0.162458, 0.119529, -0.072391, -0.200947, 0.084768, 0.213855, -0.097676, 0.033646, -0.034006, -0.054687, 0.042421);    \n"\
				"  \n"\
				"  // spline with continuity C3\n"\
				"  const mat4 a1_c3 = mat4( -0.122093, 0.081395, 0.226744, 0.313953, 0.072391, -0.119529, -0.162458, -0.106061, 0.032946, 0.005814, -0.013566, -0.025194, -0.011785, 0.005892, 0.010943, 0.007576);  \n"\
				"  const mat4 a2_c3 = mat4( 0.313953, 0.226744, 0.081395, -0.122093, 0.106061, 0.162458, 0.119529, -0.072391, -0.025194, -0.013566, 0.005814, 0.032946, -0.007576, -0.010943, -0.005892, 0.011785);  \n"\
				"  const mat4 b1_c3 = mat4( -0.122093, 0.081395, 0.226744, 0.313953, 0.072391, -0.119529, -0.162458, -0.106061, 0.032946, 0.005814, -0.013566, -0.025194, -0.011785, 0.005892, 0.010943, 0.007576);  \n"\
				"  const mat4 b2_c3 = mat4( 0.313953, 0.226744, 0.081395, -0.122093, 0.106061, 0.162458, 0.119529, -0.072391, -0.025194, -0.013566, 0.005814, 0.032946, -0.007576, -0.010943, -0.005892, 0.011785);  \n"\
				"  \n"\
				"  const vec4 QA = vec4(  -4.000000, 8.000000, -21.333333, 64.000000);\n"\
				"  const vec4 QB = vec4(   4.000000, 8.000000,  21.333333, 64.000000);\n"\
				"  \n"\
				"  const float pi1  = 3.1415;\n"\
				"  const float pi2  = 1.5708;\n"\
				"  const float pi4  = 0.7853;\n"\
				"  const float pi6	= 0.5235;\n"\
				"  const float pi12 = 0.2617;\n"\
				"  \n"\
				"  \n"\
				"float max4( in vec4 v) {\n"\
				"  float a=max(v.x,v.y);\n"\
				"  float b=max(v.z,v.w);\n"\
				"  return max(a,b);\n"\
				"}\n"\
				"  \n"\
				"  \n"\
				"  \n"\
				"float min4( in vec4 v) {\n"\
				"  float a=min(v.x,v.y);\n"\
				"  float b=min(v.z,v.w);\n"\
				"  return min(a,b);\n"\
				"}\n"\
				"  \n"\
				"  \n"\
				"  \n"\
				"float sum4( in vec4 v) {\n"\
				"  return v.x+v.y+v.z+v.w;\n"\
				"}\n"\
				"  \n"\
				"  \n"\
				"  \n"\
				"vec4 readNormalDepth( in vec2 coord ) {\n"\
				"   vec4 d = texture2D(samplerNormalDepth,coord);\n"\
				"	\n";
			if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED )
				fragShaderSS <<
				"	vec3 normal = d.y*imagePlaneLeftUniform;\n"\
				"	normal     += d.z*imagePlaneUpUniform;\n"\
				"	float      q = sqrt(1.0-d.x*d.x + d.y*d.y);\n "\
				"	normal     += q*camDirUniform;\n"\
				"	return vec4(normalize(normal),d.x);\n"\
				"	\n";
			if ( _SSAO_type != SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED )
				fragShaderSS <<
				"	return vec4(0.0,0.0,0.0,d.x);\n"\
				"	\n";
			fragShaderSS <<
				"} \n"\
				"  \n"\
				"  \n"\
				"  \n"\
				"vec4 dtParam( in vec4 p) {\n"\
				"	return vec4(p.y,2.0*p.z,3.0*p.w,0.0);\n"\
				"}\n"\
				"  \n"\
				"  \n"\
				"  \n"\
				"float polyEval( in vec4 p, in float t) {\n"\
				"	return p.x + p.y * t + p.z * pow(t,2.0) + p.w * pow(t,3.0);\n"\
				"}\n"\
				"  \n"\
				"  \n"\
				"  \n"\
				"float splineInt( in vec4 p1, in vec4 p2 ) {\n"\
				"  return dot(QB,p2)-dot(QA,p1);\n"\
				"}\n"\
				"  \n"\
				"  \n"\
				"void main(void)\n"\
				"{\n"\
				"   \n"\
				"	vec4 centerDepth = readNormalDepth(vTexCoord.xy);\n"\
				"	if ( centerDepth.w <= 0.0 ) {\n"\
				"		return;\n"\
				"	}\n"\
				"   \n"\
				"	float wFilter = 1.0-centerDepth.w/farUniform;\n"\
				"	\n"\
				"	float offsetIdx = float(mod( (gl_FragCoord.x)+(gl_FragCoord.y)+((1.0+gl_FragCoord.x)*(1.0+gl_FragCoord.y)),2.0));"
				"	\n"\
				"   \n"\
				"	float prDiff         = metricNearFarPlanePixelResolutionUniform.y - metricNearFarPlanePixelResolutionUniform.x;\n"\
				"	float prAtPOI        = metricNearFarPlanePixelResolutionUniform.x + prDiff * wFilter;\n"\
				"   float prAtPOIRadius  = 3.0*algPESSAO_RadiusScaleStepFactorUniform/textureSubSamplingFactorUniform/textureSubSamplingFactorUniform/prAtPOI;\n"\
				"   \n"\
				"	float iLoopMax = ceil(max(6.0,min(algPESSAO_LoopMaxUniform,prAtPOIRadius)) );\n"\
				"	\n"\
				"	float idx = 0.0;\n"\
				"   float AO2Write=0.0;\n"\
				"   float AO2WriteDIV=0.0;\n"\
				"	float idxMax =algPESSAO_ArtifactsRemove_CNT;\n"\
				"	float subSampling = max(0.0,idxMax-2.0) * wFilter;\n"\
				"   float angle0 = offsetIdx*pi2;\n"\
				"	\n"\
				"	//float splineCNT=0.0;\n"\
				"	\n"\
				"	while ( idx < idxMax )   \n"\
				"   {\n"\
				"		angle0 += pi2+((pi1*max(0.0,idxMax-idx-1.0)/(2.0*idxMax)));\n"\
				"		float iLoop=0.0;\n"\
				"       vec4  ndL1,ndL2,ndL3,ndL4,ndR1,ndR2,ndR3,ndR4;\n"\
				"       vec2 v1 = algPESSAO_InitRadiusInPixel*vec2(cos(angle0),sin(angle0))/textureSizeUniform;\n"\
				"		ndL1 = ndL2 = centerDepth;\n"\
				"		ndL3 = ndL4 = readNormalDepth(vTexCoord.xy-(v1));\n"\
				"		ndR1 = ndR2 = centerDepth;\n"\
				"		ndR3 = ndR4 = readNormalDepth(vTexCoord.xy+(v1));\n"\
				"		\n"\
				"		float finalAO = 0.0;\n"\
				"		float finalAOCnt = 0.0;\n"\
				"		\n";
			if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED )
				fragShaderSS <<
				"		float NB_maxAO    = 0.0;\n"\
				"		float NB_sumAO    = 0.0;\n"\
				"		float NB_sumAOCNT = 0.0;\n"\
				"       \n";
			fragShaderSS <<
				"       \n"\
				"       float currentAngle1 = angle0;\n"\
				"		float diffZDepthSum = 1.0;	\n"\
				"       float maxDiffZDepth = 0.0;\n"\
				"       \n"\
				"       while ( iLoop < iLoopMax ) \n"\
				"		{\n"\
				"			\n"\
				"			float sLoop = (iLoop+1.0) / (iLoopMax);\n"\
				"			\n"\
				"       	float angle1 = currentAngle1 + (float(mod(iLoop,4.0))*pi1/4.0);\n"
				"			currentAngle1 += ((pi2*(iLoopMax-iLoop-1.0)/(2.0*iLoopMax)));\n"\
				"			\n"\
				"			/* read next depth/normal value */ \n"\
				"			float radius  = algPESSAO_InitRadiusInPixel + pow(sin(pi2*sLoop),3.0) * prAtPOIRadius;\n"\
				"		    vec2 v4 = radius*vec2(cos(angle1),sin(angle1))/textureSizeUniform;\n"\
				"			\n"\
				"			ndL4 = readNormalDepth(vTexCoord.xy-v4);\n"\
				"			ndR4 = readNormalDepth(vTexCoord.xy+v4);\n"\
				"			\n"\
				"			\n"\
				"			/* spline interpolation : fit a spline into the surface (depth values) */\n"\
				"			vec4 z1=vec4(ndL4.w,ndL3.w,ndL2.w,ndL1.w)-centerDepth.w;\n"\
				"			vec4 z2=vec4(ndR1.w,ndR2.w,ndR3.w,ndR4.w)-centerDepth.w;\n"\
				"			\n"\
				"			/* solve ECLSQ : Mz = p */\n"\
				"			vec4 p1 = z1*a1_c2 + z2*a2_c2;\n"\
				"			vec4 p2 = z1*b1_c2 + z2*b2_c2;\n"\
				"			\n"\
				"			/* get min/max z value */\n"\
				"			float minZ = min(min4(z1),min4(z2));\n"\
				"			float maxZ = max(max4(z1),max4(z2));\n"\
				"			float diffZDepth = maxZ-minZ;\n"\
				"			diffZDepthSum += sqrt(diffZDepth);\n"\
				"			maxDiffZDepth = max(maxDiffZDepth,diffZDepth);\n"\
				"			\n"\
				"			/* integration : estimate the area ( positive / negative classification ) */\n"\
				"			/* transform the i0 value into AO estimation factor (AOEF) */\n"\
				"			float updateAO = clamp( splineInt(p1,p2)/(algPESSAO_DepthCutOffUniform_Const+8.0*(diffZDepth)*algPESSAO_DepthCutOffUniform ),0.0,1.0);\n"\
				"			updateAO *= step(farUniform*algPESSAO_DEPTH_OFFSETUniform,(max4(abs(z1-centerDepth.w))+max4(abs(z2-centerDepth.w)))/2.0);\n"\
				"    		\n"\
				"			updateAO *= step(farUniform*algPESSAO_DEPTH_OFFSETUniform,diffZDepth);\n"\
				"			float att = clamp(pow(algPESSAO_AttenuationFactor,maxDiffZDepth),0.0,1.0);\n"\
				"			updateAO *= att;\n"\
				"			\n"\
				"			/* update finalAO */\n"\
				"			float r = 1.0/(diffZDepthSum);\n"\
				"			\n"\
				"			float k = (z2.z)-(z1.y); k = 1.0/(1.0+exp(-10.0*k));r=k*r;  \n"\
				"			\n"\
				"			finalAOCnt += r;\n"\
				"			finalAO    += r*(1.0-updateAO);\n"\
				"			\n";
			if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED   )
				fragShaderSS <<
				"			/* use normal for special effects */\n"\
				"			float NB_angleDiff = dot( ndL4.xyz,ndR4.xyz);\n"\
				"			float classifier   = -dot( ( cross(ndL4.xyz,centerDepth.xyz) ) , ( cross(centerDepth.xyz,ndR4.xyz)  ) );\n"\
				"			float NB_sel       = (step(centerDepth.w,ndL4.w)*step(centerDepth.w,ndR4.w));\n"\
				"			NB_maxAO       = (1.0-att*att)*min(1.0,max( NB_sel*classifier*NB_angleDiff , NB_maxAO));\n"\
				"			float nr       = 1.0+(1.0-att*sLoop)*(1.0-finalAO/finalAOCnt);\n"\
				"			NB_sumAO      += nr*clamp(1.0-NB_maxAO,0.0,1.0);\n"\
				"			NB_sumAOCNT   += nr;\n"\
				"			\n"\
				"			\n";
			fragShaderSS <<
				"			/* update iLoop */\n"\
				"			float speedUP = max(1.0,pow(1.0+algPESSAO_SpeedUpILoop,diffZDepthSum));\n"\
				"			iLoop += speedUP;\n"\
				"           //splineCNT+=1.0;\n	"\
				"       }\n"\
				"  	  	\n"\
				"  	  	/* increase/decrease contrast*/\n"\
				"       finalAO = clamp(1.0-(1.0-pow( finalAO/finalAOCnt , contrastUniform)) ,0.0,1.0);\n"\
				"		\n";
			if ( _SSAO_type == SSAO::POLY_ESTIMATED_SSAO_NORMAL_BASED  )
				fragShaderSS <<
				"       float  NB_ao = clamp(pow(( NB_sumAO / NB_sumAOCNT),1.0-finalAO+algPESSAO_contrastUniformNB),0.0,1.0);\n"\
				"       finalAO = (finalAO * (1.0- (1.0-finalAO) * NB_ao));\n";
			fragShaderSS <<
				"		\n"\
				"		\n"\
				"		AO2Write    += abs(finalAO);\n"\
				"		AO2WriteDIV += 1.0;\n"\
				"		idx += max(1.0,ceil(  (1.0-algPESSAO_FastOff)*((idxMax*step(0.995,AO2Write/AO2WriteDIV) ) )));\n"\
				"	}\n"\
				"	float aoReturn = AO2Write/AO2WriteDIV;\n"\
				"		\n"\
				"   //gl_FragData[0] = vec4( 1.0/(1.0+splineCNT)*aoReturn,centerDepth.w,0.0,1.0);\n"\
				"   gl_FragData[0] = vec4( aoReturn,centerDepth.w,wFilter,1.0);\n"\
				"}\n";
			osg::ref_ptr<osg::Shader> fragment_shader = new osg::Shader(osg::Shader::FRAGMENT,fragShaderSS.str());
			program->addShader(fragment_shader.get());
			addPass(ss.get());

			//std::cout << fragShaderSS.str() << std::endl;
		} // SSAO::POLY_ESTIMATED_SSAO

	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// PASS 3 : Shadow Pass
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	{   // shadow Pass
		osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
		addPass(ss.get());
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/// PASS 4 : Final Pass
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	{   // Final Pass
		osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
		addPass(ss.get());
	}
}


SSAO::SSAO(osg::Group* rootGrp,
		   osg::LightSource* ls,
		   std::string lightMap
		   )  :
Effect(),
_LightSource(ls),
_lightMap(lightMap)
{

	osg::ref_ptr<osg::LightSource> lightSrc0 = new osg::LightSource;
	osg::ref_ptr<osg::LightSource> lightSrc1 = new osg::LightSource;

	lightSrc0->setName("Temporary Light Source 0 for SSAO ");
	lightSrc1->setName("Temporary Light Source 1 for SSAO ");


	osg::ref_ptr<osg::Light> light0 = lightSrc0->getLight();
	osg::ref_ptr<osg::Light> light1 = lightSrc1->getLight();

	light0->setLightNum(0);
	light1->setLightNum(1);


	osg::Vec4 pos; 
	osg::Vec3 dir; 	
	osg::ref_ptr<osg::Light> light;

	for ( unsigned int iLoop=0;iLoop<6;iLoop++) {
		osg::Vec4 ambient ( 0.0, 0.0, 0.0, 0.0);
		osg::Vec4 diffuse ( 0.0, 0.0, 0.0, 0.0);
		osg::Vec4 specular( 0.0, 0.0, 0.0 ,0.0);
		switch(iLoop) {
case 0: {pos.set(1.0,0.0,0.0,0.0);dir.set(-1.0,0.0,0.0);light = light0.get(); break;}
case 1: {pos.set(0.0,1.0,0.0,0.0);dir.set(0.0,-1.0,0.0);light = light1.get();break;}
default: break;
		}

		light->setPosition(pos);
		light->setDirection(dir);

		light->setAmbient(ambient);
		light->setDiffuse(diffuse);
		light->setSpecular(specular);
		light->setConstantAttenuation(1.0);
		light->setLinearAttenuation(0.0);
		light->setQuadraticAttenuation(0.0);
		light->setSpotExponent(0.0);
		light->setSpotCutoff(180.0);
	}



	_root = new osg::Group;
	osg::ref_ptr<osg::StateSet> stateSetRoot = _root->getOrCreateStateSet();
	{
		stateSetRoot->setMode(GL_LIGHTING,osg::StateAttribute::ON);
		stateSetRoot->setMode(GL_LIGHT0,osg::StateAttribute::ON);
		stateSetRoot->setMode(GL_LIGHT1,osg::StateAttribute::ON);
	}
	_root->addChild(lightSrc0.get());
	lightSrc0->addChild(lightSrc1.get());
	lightSrc1->addChild(rootGrp);

}


SSAO::SSAO(const SSAO& copy, const osg::CopyOp& copyop)   :    Effect(copy, copyop){
}

bool SSAO::define_techniques()
{
	addTechnique(new DefaultTechnique(_root.get(),this,this->_ssao_type,_LightSource.get(),_lightMap));
	return true;
}


                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      