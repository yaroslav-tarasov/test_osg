#pragma once

//
// Lightning for sky dome layer
//
namespace avSky
{

class LightningLayer : public osg::Geode
{
public:


public:

    // constructor
    LightningLayer(osg::Group * sceneRoot);

    // set clouds 2 color scheme
    void setColors( const osg::Vec3f & vFrontColor, const osg::Vec3f & vBackColor );

	// set density
    bool setDensity( float density );

    // set rotation by sidereal time
    void setRotationSiderealTime( float rot_deg );

    // get overcast coefficient
    float getDensity() const {return _clDensity;} 

private:
	bool loadTextures();

private:

    // scene ptr
    osg::Group * _pScenePtr;

    osg::ref_ptr<osg::Uniform> _mvpUniform;
    osg::ref_ptr<osg::Uniform> _colorFrontUniform;
    osg::ref_ptr<osg::Uniform> _colorBackUniform;
    osg::ref_ptr<osg::Uniform> _densityUniform;
    
    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();

private:

    // input
    float        _clDensity;
    osg::Matrixf _clRotation;

};

}