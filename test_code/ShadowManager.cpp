#include <ShadowManager/ShadowManager.h>
#include <dtCore/light.h>

#include <assert.h>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>

#include <osgShadow/SoftShadowMap>
#include <osg/LightSource>

#include <dtCore/scene.h>
#include <dtGame/gamemanager.h>
#include <dtUtil/log.h>

//////////////////////////////////////////////////////////////////
// fragment shader
// Uber hackerific
static const char fragShaderSource[] = 
"uniform sampler2D osgShadow_baseTexture; n"
"uniform sampler2DShadow osgShadow_shadowTexture; n"
"uniform vec2 osgShadow_ambientBias; n"
"n"
"void main(void) n"
"{ n"
"    vec4 ambient = vec4(0.3, 0.3, 0.3, 0.0); n"
"    vec4 color = (gl_Color + ambient) * texture2D( osgShadow_baseTexture, gl_TexCoord[0].xy ); n"
"    gl_FragColor = color * (osgShadow_ambientBias.x + shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[1] ) * osgShadow_ambientBias.y); n"
"}n";

////////////////////////////////////////////////////////////////////////////////
ShadowManager::ShadowManager(dtGame::GameManager* gameManager, 
                             const osg::Vec3& lightDirection,
                             eShadowResolution res /*= SR_HIGH*/,
                             float minLightMargin /* 10.0f */,
                             float maxFarPlane /* = 1024.0f */,
                             ShadowMapType shadowMapType /* = LSPSM */ )
   : mpGameManager(gameManager)
   , mLightSource(NULL)
   , mResolution(res)
   , mMinLightMargin(minLightMargin)
   , mMaxFarPlane(maxFarPlane)
   , mShadowMapType(shadowMapType)
{
   osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
   shadowedScene->setReceivesShadowTraversalMask(SHADOW_RECEIVE_NODE_MASK);
   shadowedScene->setCastsShadowTraversalMask(SHADOW_CAST_NODE_MASK);

   gameManager->GetScene().SetSceneNode(shadowedScene.get());

   mLightSource = new osg::LightSource;   
   mLightSource->getLight()->setPosition(osg::Vec4(-lightDirection, 0.0f));
   mLightSource->getLight()->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

   assert(gameManager->GetScene().GetLight(0));
   gameManager->GetScene().GetLight(0)->SetAmbient(1.0f, 1.0f, 1.0f, 1.0f);

   SetShadowMap();
}

////////////////////////////////////////////////////////////////////////////////
void ShadowManager::Update()
{
   // todo -- update light position

   // extract environment
   dtGame::IEnvGameActorProxy* envactorproxy = mpGameManager->GetEnvironmentActor();
   envactorproxy;
}

////////////////////////////////////////////////////////////////////////////////
void ShadowManager::SetShadowsEnabled(bool enabled)
{
   if (enabled)
   {
      SetShadowMap();
   }
   else
   {
      osgShadow::ShadowedScene* scene = dynamic_cast<osgShadow::ShadowedScene*>(mpGameManager->GetScene().GetSceneNode());
      assert(scene);

      scene->setShadowTechnique(NULL);
      scene->removeChild(mLightSource.get());
   }
}

////////////////////////////////////////////////////////////////////////////////
void ShadowManager::SetShadowMap()
{
   int shadowRes = 0;
   if (mResolution == SR_HIGH)        { shadowRes = 4096; }
   else if (mResolution == SR_MEDIUM) { shadowRes = 2048; }
   else if (mResolution == SR_LOW)    { shadowRes = 1024; }

   osg::ref_ptr<osgShadow::ShadowTechnique> shadowMap;

   if (mShadowMapType == BASIC_SHADOW_MAP)
   {
      /*osg::ref_ptr<osgShadow::ShadowMap>*/ shadowMap =
         GetBasicShadowMap(shadowRes);
   }
   else if (mShadowMapType == LSPSM)
   {
      /*osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapCB>*/ shadowMap =
         GetLightSpacePerspectiveShadowMap(shadowRes);
   }
   else if (mShadowMapType == SOFT_SHADOW_MAP)
   {
      /*osg::ref_ptr<osgShadow::SoftShadowMap>*/ shadowMap =
         GetSoftShadowMap(shadowRes);
   }  
   else if (mShadowMapType == PSSM)
   {
      /*osg::ref_ptr<osgShadow::ParallelSplitShadowMap>*/ shadowMap =
         GetParallelSplitShadowMap(shadowRes);
   }
   else
   {
      LOG_ERROR("Unrecognized Shadow Map Type.");
      return;
   }

   // The scene needs to be a shadowed scene for shadows to work correctly
   osgShadow::ShadowedScene* scene = dynamic_cast<osgShadow::ShadowedScene*>(mpGameManager->GetScene().GetSceneNode());
   assert(scene);

   scene->setShadowTechnique(shadowMap.get());
   scene->addChild(mLightSource.get());
}

////////////////////////////////////////////////////////////////////////////////
osgShadow::ShadowMap* ShadowManager::GetBasicShadowMap(int shadowRes)
{
   osg::Shader* fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragShaderSource);   

   // Setup the scene with shadows via shadow mapping
   osgShadow::ShadowMap* shadowMap = new osgShadow::ShadowMap;
   shadowMap->setTextureSize(osg::Vec2s(shadowRes, shadowRes));
   shadowMap->setLight(mLightSource.get());
   shadowMap->addShader(fragShader);
   //shadowMap->setAmbientBias(osg::Vec2(1.0f, 10.1f));

   return shadowMap;
}

////////////////////////////////////////////////////////////////////////////////
osgShadow::LightSpacePerspectiveShadowMapCB* ShadowManager::GetLightSpacePerspectiveShadowMap(int shadowRes)
{
   osgShadow::LightSpacePerspectiveShadowMapCB* shadowMap = 
      new osgShadow::LightSpacePerspectiveShadowMapCB;
   
   unsigned int baseTexUnit = 0;
   unsigned int shadowTexUnit = 1; 

   shadowMap->setMinLightMargin(mMinLightMargin);
   shadowMap->setMaxFarPlane(mMaxFarPlane);
   shadowMap->setTextureSize(osg::Vec2s(shadowRes, shadowRes));
   shadowMap->setShadowTextureCoordIndex(shadowTexUnit);
   shadowMap->setShadowTextureUnit(shadowTexUnit);
   shadowMap->setBaseTextureCoordIndex(baseTexUnit);
   shadowMap->setBaseTextureUnit(baseTexUnit);

   return shadowMap;
}

////////////////////////////////////////////////////////////////////////////////
osgShadow::SoftShadowMap* ShadowManager::GetSoftShadowMap(int shadowRes)
{
   osgShadow::SoftShadowMap* shadowMap = new osgShadow::SoftShadowMap;
   shadowMap->setTextureSize(osg::Vec2s(2048, 2048));
   shadowMap->setSoftnessWidth(1.0f);   

   return shadowMap;
}

////////////////////////////////////////////////////////////////////////////////
osgShadow::ParallelSplitShadowMap* ShadowManager::GetParallelSplitShadowMap(int shadowRes)
{
   osgShadow::ParallelSplitShadowMap* shadowMap = new osgShadow::ParallelSplitShadowMap(NULL, 3);
   shadowMap->setTextureResolution(1024);
   shadowMap->setMinNearDistanceForSplits(0.25);
   shadowMap->setMaxFarDistance(1024.0);
   shadowMap->setPolygonOffset(osg::Vec2(10.0f, 20.0f));

   return shadowMap;
}

////////////////////////////////////////////////////////////////////////////////
