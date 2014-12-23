#pragma once

 namespace avTerrain
 {
     class Terrain : public osg::Group
     {
     public:
         Terrain (osg::Group* sceneRoot);
         /*osg::Node* */ 
         void create( std::string name );
     protected:
         void fill_navids(std::string file, std::vector<osg::ref_ptr<osg::Node>>& cur_lamps, osg::Group* parent, osg::Vec3f const& offset);
     private:
         osg::ref_ptr<osg::Group> _sceneRoot;
          
                      
     };

 }