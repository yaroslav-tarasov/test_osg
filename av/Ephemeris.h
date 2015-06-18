#pragma once


namespace avSky
{


    class Ephemeris : public osg::Group
    {
       friend class EphemerisDataUpdateCallback;
    public:       
        
        typedef std::function<void(float)>  on_illum_change_f;
        
    public:
       Ephemeris(osg::Group * sceneRoot,osg::Group * terrainNode,on_illum_change_f ic = nullptr, on_visible_range_change_f vc = nullptr);
       bool			           Initialize();
       void                    setSkyDomeRadius(double radius);
       void                    setSunLightSource(osg::LightSource* ls);
       osg::LightSource*       getSunLightSource();
       osgGA::GUIEventHandler* getEventHandler();
       inline  float           getIllumination() const {return _illum;};

    protected:      
       void                    setTime();
       void                    setSummerTime();
       inline  void            setIllumination(float illum) { if(_ic) _ic(illum); _illum = illum; }
       void                    setStarFieldMask(osg::Node::NodeMask nm);  
       void                    cull( osg::NodeVisitor * pNV );
       osg::Matrixd            getModelViewMatrix() {return _mv; };         

    private:
        struct data;
        osg::ref_ptr<data>             _d;
        osg::ref_ptr<osg::Node>        _sceneRoot;  
        osg::ref_ptr<osg::Node>        _terrainNode;
        float                          _illum; 
        on_illum_change_f              _ic;
        on_visible_range_change_f      _vc;
        osg::ref_ptr<osg::Uniform>     _specularUniform;
        osg::ref_ptr<osg::Uniform>     _ambientUniform;
        osg::ref_ptr<osg::Uniform>     _diffuseUniform;
        osg::ref_ptr<osg::Uniform>     _lightDirUniform; 
        osg::Matrixd                   _mv; 
    };
}