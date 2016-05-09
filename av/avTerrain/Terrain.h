#pragma once

#include "av/avLights/Lights.h"

namespace utils
{
    struct  LoadNodeThread;
}


 namespace avTerrain
 {
     class Terrain : public osg::Group
     {
     public:
         Terrain (osg::Group *);
         void Create( const std::string& name );
		 void setGrassMapFactor(float value);

     private:
         void cull( osg::NodeVisitor * pNV );
     private:
         osg::Group*                    _sceneRoot;
         utils::LoadNodeThread*         _lnt;

         // Dynamic lights handler
         avScene::LightNodeHandler      _lightsHandler;

         osg::ref_ptr<osg::Group>       _grass;
         
     };

 }