/**
* Adrian Egli (2010)
* OpenSceneGraph -> Screen Space Ambient Occlusion
*                   Cubic Spline Based Ambient Occlusion
**/

#ifndef __FFSSAO_Header_
#define __FFSSAO_Header_

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgUtil/CullVisitor>

#include <osg/CoordinateSystemNode>
#include <osg/Switch>
#include <osg/Uniform>

#include <osgText/Text>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/BlendFunc>
#include <osg/LightSource>

#include <osgGA/GUIEventHandler>

#include <iostream>

#include <osgFX/Effect>
#include <osgFX/Technique>
#include <osgFX/Registry>

#include <osg/TexGen>
#include <osg/Texture2D>

#include <osg/PolygonOffset>
#include <osgDB/WriteFile>


namespace osgFX {
	class SSAO : public Effect {
		public:
			enum SSAO_TYPE {
				DEFAULT_SSAO=0,
				FFNBSSAO=1,
 				POLY_ESTIMATED_SSAO=2,
				POLY_ESTIMATED_SSAO_NORMAL_BASED=3
			} ;


			SSAO() { _ssao_type = POLY_ESTIMATED_SSAO; };
			SSAO(osg::Group*,osg::LightSource* ls=NULL, std::string lightMap="lightmap.png");
			SSAO(const SSAO& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

			META_Effect(osgFX, SSAO,

				"Screen Space Ambient Occlusion",

				"SSAO : test implementation",

				"Adrian Egli");



		protected:
			virtual ~SSAO() {}
			SSAO& operator=(const SSAO&) { return *this; }

			bool define_techniques();

			osg::ref_ptr<osg::Group> _root;

		public:
			static float global_max_phi;
			static float global_max_radius;
			static float global_contrast;
			static float global_algPESSAO_contrastNB;
			static float global_algPESSAO_LoopMax;
			static float global_algPESSAO_DEPTH_OFFSET;
			static float global_algPESSAO_COLOR_BLEND;
			static float global_algPESSAO_CamMoveAO;
			static float global_algPESSAO_RadiusScaleStepFactor;
			static float global_algPESSAO_DepthCutOff;
			static float global_algPESSAO_DepthCutOff_Const;
			static float global_algPESSAO_InitRadiusInPixel;
			static float global_algPESSAO_FastOff;
			static float global_algPESSAO_ArtifactsRemove_CNT;
			static float global_algPESSAO_SpeedUpILoop;
 			static float global_algPESSAO_AttenuationFactor;
 			static bool global_debug_color_pssm;

			static int global_shadow_show;

			static int WIDTH;
			static int HEIGHT;
			static int MAX_WIDTH;
			static int MAX_HEIGHT;

			static float SUB_SAMPLING_FACTOR;

			SSAO_TYPE _ssao_type;

			osg::ref_ptr<osg::LightSource> _LightSource;
			std::string _lightMap;


	};

}



using namespace osgFX;


namespace
{

	class DefaultTechnique: public Technique {
	public:

		DefaultTechnique(osg::Group* scene,osg::Group* ssaoFX, SSAO::SSAO_TYPE type, osg::LightSource* lightSource, std::string lightMap);

		  virtual void getRequiredExtensions(std::vector<std::string>& extensions);

		  bool validate(osg::State& state) const;

		  /** optional: return a node that overrides the child node on a specified pass */
		  inline virtual osg::Node* getOverrideChild(int passNum);
		  virtual void traverse(osg::NodeVisitor& nv, Effect* fx);

	private:
		  void createRTTInfoObjects(unsigned int viewportWidth, unsigned int viewportHeight);

		  int   _viewportWidth;
		  int   _viewportHeight;
		  float _subSamplingFactor;
	protected:

		void define_passes();

		osg::Vec3 _cameraPos;
		osg::Vec3 _cameraDir;

	public:
		struct RTTTexture2Bind {
			osg::ref_ptr<osg::StateSet> _stateSet2Bind;
			unsigned int                _textureID;
		};
		struct RTTInfo {
			// RTT
			osg::ref_ptr<osg::Camera>       _camera;
			osg::ref_ptr<osg::TexGen>       _texgen;
			osg::ref_ptr<osg::Texture2D>    _texture;
			osg::ref_ptr<osg::StateSet>     _stateset;

			osg::ref_ptr<osg::Node>		    _baseTexture2DNode;

			RTTTexture2Bind				    _RTTTexture2Bind;

 			unsigned int					_resolution_x;
			unsigned int					_resolution_y;
		};

		RTTInfo						_RTTInfoNormalDepth;
 		RTTInfo						_RTTInfoSSAO;
		RTTInfo						_RTTInfoShadow;

		osg::ref_ptr<osg::Group> _scene;

		osg::ref_ptr<osg::Uniform> _nearUniform;
		osg::ref_ptr<osg::Uniform> _farUniform;

		osg::ref_ptr<osg::Uniform> _imagePlaneLeft;
		osg::ref_ptr<osg::Uniform> _imagePlaneUp;


		osg::ref_ptr<osg::Uniform> _contrastUniform;

		osg::ref_ptr<osg::Uniform> _phiStepUniform;
		osg::ref_ptr<osg::Uniform> _radiusMaxUniform;



		osg::ref_ptr<osg::Uniform> _mvpwInvUniform;

		osg::ref_ptr<osg::Uniform> _textureSizeUniform;
		osg::ref_ptr<osg::Uniform> _textureSubSamplingFactor;

		osg::ref_ptr<osg::Uniform> _camDirUniform;
		osg::ref_ptr<osg::Uniform> _camPosUniform;
 		osg::ref_ptr<osg::Uniform> _lightDirUniform;
		osg::ref_ptr<osg::Uniform> _camAspectRatioUniform;
		osg::ref_ptr<osg::Uniform> _metricNearFarPlanePixelResolutionUniform;

 		osg::ref_ptr<osg::Uniform> _algPESSAO_DEPTH_OFFSETUniform;
		osg::ref_ptr<osg::Uniform> _algPESSAO_LoopMaxUniform;
		osg::ref_ptr<osg::Uniform> _algPESSAO_COLOR_BLENDUniform;
		osg::ref_ptr<osg::Uniform> _algPESSAO_CamMoveAOUniform;
		osg::ref_ptr<osg::Uniform> _algPESSAO_InitRadiusInPixel;
		osg::ref_ptr<osg::Uniform> _algPESSAO_RadiusScaleStepFactorUniform;
		osg::ref_ptr<osg::Uniform> _algPESSAO_DepthCutOffUniform;
		osg::ref_ptr<osg::Uniform> _algPESSAO_DepthCutOffUniform_Const;
		osg::ref_ptr<osg::Uniform> _algPESSAO_contrastUniformNB;
		osg::ref_ptr<osg::Uniform> _algPESSAO_FastOff;
		osg::ref_ptr<osg::Uniform> _algPESSAO_ArtifactsRemove_CNT;
		osg::ref_ptr<osg::Uniform> _algPESSAO_SpeedUpILoop;
		osg::ref_ptr<osg::Uniform> _algPESSAO_AttenuationFactor;

		osg::ref_ptr<osg::LightSource> _LightSource;

		SSAO::SSAO_TYPE _SSAO_type;

		osg::ref_ptr<osg::Switch> _show_shadow_node;

		bool _debug_color_pssm;

	};

}



#endif