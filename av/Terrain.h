#pragma once

#include "Lights.h"

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
         /*osg::Node* */ 
         void create( const std::string& name );
		 void setGrassMapFactor(float value);
     //protected:
     //    void fill_navids(std::string file, std::vector<osg::ref_ptr<osg::Node>>& cur_lamps, osg::Group* parent, osg::Vec3f const& offset);
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