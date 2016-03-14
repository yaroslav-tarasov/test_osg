#pragma once

//
// Lightning for sky dome layer
//
namespace avSky
{

class LightningLayer : public osg::Geode
{
public:

    // constructor
    LightningLayer(osg::Group * sceneRoot);

    // set clouds 2 color scheme
    void setColors( const osg::Vec3f & vFrontColor, const osg::Vec3f & vBackColor );

	// set density
    bool setDensity( float density );

    // set rotation by sidereal time
    void setRotation( float rot_deg );

    // get overcast coefficient
    float getDensity() const {return _clDensity;} 
    
    void flash();

private:

    // update callback
    void update( osg::NodeVisitor * nv );

private:
    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();

    bool _loadTextures();

private:

    // scene ptr
    osg::Group * _pScenePtr;

    osg::ref_ptr<osg::Uniform> _mvpUniform;
    osg::ref_ptr<osg::Uniform> _colorFrontUniform;
    osg::ref_ptr<osg::Uniform> _colorBackUniform;
    osg::ref_ptr<osg::Uniform> _densityUniform;
    osg::ref_ptr<osg::Uniform> _flashUniform;

private:

    // input
    float        _clDensity;
    osg::Matrixf _clRotation;

    float        _flashTargetTime;
    bool         _flash;
};

}