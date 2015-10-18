#pragma once

namespace avSky
{

//
// Post-applied fog sky dome layer
//

class FogLayer : public osg::Geode
{
public:

    // constructor
    FogLayer(osg::Group * sceneRoot);

    // set fog parameters
    void setFogParams( const osg::Vec3f & vFogColor, float fFogDensity );
    // set under water color
    void SetUnderWaterColor( const osg::Vec3f & cUnderWaterColor );

    // get real evaluated visible distance
    float getVisDistance() const { return m_fRealVisDist; }

private:

    // scene ptr
    osg::Group * _pScenePtr;


    // fog related global uniforms
    osg::ref_ptr<osg::Uniform> _sceneFogUniform;
    osg::ref_ptr<osg::Uniform> _reflFogUniform;

    // current uniforms
    osg::ref_ptr<osg::Uniform> _skyFogUniform;
    osg::ref_ptr<osg::Uniform> _underWaterLowColorUniform;

    // real vis distance
    float m_fRealVisDist;

    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();
};

}
