#ifndef SHADOW_MANAGER__H
#define SHADOW_MANAGER__H

////////////////////////////////////////////////////////////////////////////////

#include <osg/ref_ptr>
#include <osg/Vec3>

#include <osg/LightSource>

namespace dtGame
{
   class GameManager;
}

namespace osgShadow
{
   class ShadowMap;
   class LightSpacePerspectiveShadowMapCB;
   class SoftShadowMap;
   class ParallelSplitShadowMap;
}

////////////////////////////////////////////////////////////////////////////////

class ShadowManager
{
public:   
   
   // Unfortunately osg doesn't currently repsect the shadow receive mask
   static const int SHADOW_RECEIVE_NODE_MASK = 0x1;
   static const int SHADOW_CAST_NODE_MASK = 0x2;
   static const int SHADOW_ABSTAIN_MASK = ~(SHADOW_RECEIVE_NODE_MASK | SHADOW_CAST_NODE_MASK);

   enum eShadowResolution
   {
      SR_HIGH,
      SR_MEDIUM,
      SR_LOW
   };

   enum ShadowMapType
   {
      BASIC_SHADOW_MAP,
      LSPSM,                 
      PSSM,
      SOFT_SHADOW_MAP
   };

   ShadowManager(dtGame::GameManager* gameManager, 
                 const osg::Vec3& lightDirection,
                 eShadowResolution res = SR_HIGH,
                 float minLightMargin = 10.0f,
                 float maxFarPlane = 1024.0f,
                 ShadowMapType shadowMapType = LSPSM);

   void Update(); // update light position
   void SetShadowsEnabled(bool enabled);

private:
   void SetShadowMap();
   osgShadow::ShadowMap* GetBasicShadowMap(int shadowRes);
   osgShadow::LightSpacePerspectiveShadowMapCB* GetLightSpacePerspectiveShadowMap(int shadowRes);
   osgShadow::SoftShadowMap* GetSoftShadowMap(int shadowRes);
   osgShadow::ParallelSplitShadowMap* GetParallelSplitShadowMap(int shadowRes);

   dtGame::GameManager*            mpGameManager;
   osg::ref_ptr<osg::LightSource>  mLightSource;
   eShadowResolution               mResolution;
   
   float                           mMinLightMargin;
   float                           mMaxFarPlane;

   int                             mShadowMapType;
};

////////////////////////////////////////////////////////////////////////////////

#endif // SHADOW_MANAGER__H
