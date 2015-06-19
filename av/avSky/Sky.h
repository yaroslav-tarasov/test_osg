#pragma once

namespace avSky
{
   class CloudsLayer;
}

namespace avSky
{

// forward declarations
class EphemerisModel;


class Sky : public osg::Group
{
public:
    Sky( osg::Group* pScene );

    inline EphemerisModel * GetSkydome() { return _cSkydomePtr.get(); }
    inline avSky::CloudsLayer *         GetClouds () { return _cCloudsPtr.get(); }

    float              GetSunIntensity() const;
    const osg::Vec3f & GetFogColor    () const;

    void SetWaterColor( const osg::Vec3f & cWaterColor );
    osg::LightSource* getSunLightSource();

private:

    osg::ref_ptr<EphemerisModel> _cSkydomePtr;
    osg::ref_ptr<avSky::CloudsLayer>         _cCloudsPtr;
};
}

