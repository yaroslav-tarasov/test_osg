#pragma once

namespace avSky
{

//
// Moon sphere
//

// Moon class
class MoonSphere : public osg::Geode
{
public:

    // constructor
    MoonSphere();

    // skydome direction
    void setDirection( float fAzimuth, float fElevation );
    // moon color
    void setMoonReflColor( const osg::Vec3f & cMoonColor );

protected:

    // control unifroms
    osg::ref_ptr<osg::Uniform> m_moonVectorUniform;

    // build geode and state-set
    void _buildGeometry();
};

}
