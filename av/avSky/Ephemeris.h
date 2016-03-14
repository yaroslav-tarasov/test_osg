#pragma once

#include   "av/ISky.h"

namespace avSky
{


    class Ephemeris : public osg::Group , public ISky
    {
       friend class EphemerisDataUpdateCallback;
        
    public:
       Ephemeris(osg::Group * sceneRoot,osg::Group * terrainNode);

       bool			           Initialize       ();
       void                    setSkyDomeRadius (double radius);
       void                    setSunLightSource(osg::LightSource* ls);
       osg::LightSource*       getSunLightSource();
       osgGA::GUIEventHandler* getEventHandler  ();
       inline  float           getIllumination  () const {return _illum;};
    
    //
    //  ISky
    //
    public:
       float                   GetSunIntensity() const override;
       const osg::Vec3f &      GetFogColor    () const override;

    public:
       void                    setTime         ();
       void                    setSummerTime   ();
    protected:
       void                    setIllumination ( float illum );
       void                    setStarFieldMask( osg::Node::NodeMask nm );  
       void                    cull            ( osg::NodeVisitor * pNV );
       osg::Matrixd            getModelViewMatrix() {return _mv; };         

    private:
        struct data;
        osg::ref_ptr<data>             _d;
        osg::ref_ptr<osg::Node>        _sceneRoot;  
        osg::ref_ptr<osg::Node>        _terrainNode;
        osg::ref_ptr<osg::Uniform>     _specularUniform;
        osg::ref_ptr<osg::Uniform>     _ambientUniform;
        osg::ref_ptr<osg::Uniform>     _diffuseUniform;
        osg::ref_ptr<osg::Uniform>     _lightDirUniform; 
        osg::Matrixd                   _mv; 
        float                          _illum; 
    private:
        float                           _flash_last;
    };
}