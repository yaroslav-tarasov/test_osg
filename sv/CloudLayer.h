#pragma once

//
// Post-applied fog sky dome layer
//
namespace avSky
{

class CloudsLayer : public osg::Geode
{
public:
    enum cloud_type
    {
        none,
        cloudy,
        cirrus,
        overcast,
        clouds_types_num
    } clouds_type;

public:

    // constructor
    CloudsLayer(osg::Group * sceneRoot);

    // set clouds 2 color scheme
    void setCloudsColors( const osg::Vec3f & vFrontColor, const osg::Vec3f & vBackColor );
    // set clouds texture
    bool setCloudsTexture( cloud_type cl_type );
    // set density
    bool setCloudsDensity( float density );

    // set rotation by sidereal time
    void setRotationSiderealTime( float rot_deg );

    // get overcast coefficient
    float getOvercastCoef()  const;
    float getCloudsDensity() const {return _clDensity;} 

private:

    // scene ptr
    osg::Group * _pScenePtr;

    osg::ref_ptr<osg::Uniform> _mvpUniform;
    osg::ref_ptr<osg::Uniform> _cloudsColorFrontUniform;
    osg::ref_ptr<osg::Uniform> _cloudsColorBackUniform;
    osg::ref_ptr<osg::Uniform> _cloudsDensityUniform;
    
    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();

private:

    // input
    float        _clDensity;
    cloud_type   _clType;
    osg::Matrixf _clRotation;

};

}