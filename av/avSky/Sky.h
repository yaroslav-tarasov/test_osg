#pragma once

#include   "av/ISky.h"

//namespace avSky
//{
//   class CloudsLayer;
//}

namespace avSky
{

// forward declarations
class EphemerisModel;


class Sky : public osg::Group , public ISky
{
public:
    Sky( osg::Group* pScene );

    inline EphemerisModel * GetSkydome() { return _cSkydomePtr.get(); }
    //inline avSky::CloudsLayer *         GetClouds () { return _cCloudsPtr.get(); }

    float              GetSunIntensity() const override;
    const osg::Vec3f & GetFogColor    () const override;

    void SetWaterColor( const osg::Vec3f & cWaterColor );
    osg::LightSource* getSunLightSource();
    

private:

    osg::ref_ptr<EphemerisModel> _cSkydomePtr;
    // osg::ref_ptr<avSky::CloudsLayer>         _cCloudsPtr;
};
}

