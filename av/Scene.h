#pragma once 


#include "application/panels_fwd.h"

//
//  Forwards
//

namespace avSky
{
    class Ephemeris;
}

namespace avTerrain
{
    class Terrain;
}

namespace avShadow
{
    class ShadowTechnique;
}

namespace bi
{
    class RigidUpdater;
}

class PickHandler;


namespace avScene {

    //
    // Scene class itself
    //

    class Scene : public osg::Group
    {
    public:
        static bool                                 Create( osg::ArgumentParser& cArgs ,osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr = NULL );
        static void                                 Release();
        static Scene*                               GetInstance();
        
        //FIXME TODO // virtual bool                FrameCall();

        inline osgViewer::Viewer*                   GetViewer();
        inline osg::Group*                          getEphemerisNode();
        inline osg::Group*                          getCommonNode();
        inline osg::Camera*                         getCamera();
        
        osg::Node*                                  load(std::string path, osg::Node* parent=0, uint32_t seed=0);

        static std::string                          zoneToReload(){return zone_to_reload_;}
    private:

        Scene();
        virtual									    ~Scene();
        bool									    Initialize( osg::ArgumentParser& cArgs,osg::ref_ptr<osg::GraphicsContext::Traits> cTraitsPtr, int nWidth = 0, int nHeight = 0);
        void                                        createTerrainRoot();
        void                                        createObjects();

    private:
        void                                        onZoneChanged(int zone);
        bool                                        onEvent( const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa, osg::Object * obj, osg::NodeVisitor * nv );
    private:

        static osg::ref_ptr<Scene>                  _scenePtr;
        osg::ref_ptr<osgViewer::Viewer>             _viewerPtr;
        osg::ref_ptr<osg::Group>                    _commonNode;
        osg::ref_ptr<avSky::Ephemeris>              _ephemerisNode;
        osg::ref_ptr<osg::Group>                    _weatherNode;
        osg::ref_ptr<avTerrain::Terrain>            _terrainNode; 
        osg::ref_ptr<osg::LightSource>              _ls;
        osg::ref_ptr<osg::Group>                    _terrainRoot;
        osg::ref_ptr<avShadow::ShadowTechnique>     _st;  
        osg::ref_ptr<bi::RigidUpdater>              _rigidUpdater;
        osg::ref_ptr<PickHandler>                   _pickHandler; 
        std::vector<osg::ref_ptr<osg::Node>>        _lamps;

        app::vis_settings_panel_ptr                 _vis_settings_panel;
        connection_holder                           conn_holder_;

        static std::string                           zone_to_reload_;

    };

    inline osgViewer::Viewer*                   Scene::GetViewer() {  return _viewerPtr; }
    inline osg::Camera*                         Scene::getCamera() {  return _viewerPtr->getCamera(); }
    inline osg::Group*                          Scene::getCommonNode() { return _commonNode.get(); }  

    inline Scene* GetScene()
    {
        return Scene::GetInstance();
    }

} // end namespace