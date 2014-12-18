#pragma once

 namespace avTerrain
 {
     class Terrain : public osg::Group
     {
     public:
         Terrain (osg::Group* sceneRoot);
         /*osg::Node* */ 
         void create( std::string name );
     private:
         osg::ref_ptr<osg::Group> _sceneRoot;

                      
     };

     namespace bi
     {
        osg::ref_ptr<osgGA::GUIEventHandler>& getUpdater();
     }
 }