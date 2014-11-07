#pragma once

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

    // get real evaluated visible distance
    float getVisDistance() const { return m_fRealVisDist; }

    // get fog coefficient
    float getFogExp2Coef() const { return m_realExp2Density; }

private:

    // scene ptr
    osg::Group * _pScenePtr;


    // fog related global uniforms
    osg::ref_ptr<osg::Uniform> _sceneFogUniform;

    // current uniforms
    osg::ref_ptr<osg::Uniform> _skyFogUniform;

    // real vis distance
    float m_fRealVisDist;
    float m_realExp2Density;

    // create geometry
    void _createGeometry();
    // create state set
    void _buildStateSet();
};

